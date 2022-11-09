﻿#include "TriangleApp.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <chrono>

#include "MovementController.h"
#include "SimpleRenderSystem.h"

namespace svk
{
    struct GlobalUbo {
        alignas(16) glm::mat4 projection{1.0f};
        alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
    };
    
    TriangleApp::TriangleApp() {
        globalPool = DescriptorPool::Builder(device).setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT).build();
        loadGameObjs();
    }
    
    TriangleApp::~TriangleApp() { }

    void TriangleApp::run() {
        std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); ++i) {
            uboBuffers[i] = std::make_unique<Buffer>(device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build();

        std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); ++i) {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            DescriptorWriter(*globalSetLayout, *globalPool).writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
        }
        
        Buffer gloablUboBuffer{device, sizeof(GlobalUbo), SwapChain::MAX_FRAMES_IN_FLIGHT,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            device.properties.limits.minUniformBufferOffsetAlignment};
        gloablUboBuffer.map();
        
        SimpleRenderSystem simpleRenderSystem{device, renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
        Camera camera{};

        auto viewerObject = GameObj::createGameObj();
        MovementController cameraController{};
        auto currentTime = std::chrono::high_resolution_clock::now();
        
        while (isRunning) {
            SDL_Event e;
            cameraController.refresh();
            while (SDL_PollEvent(&e)) {
                switch (e.type) {
                case SDL_WINDOWEVENT: {
                    switch (e.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED: 
                        //recreateSwapChain();
                        break;
                    case SDL_WINDOWEVENT_CLOSE: 
                        e.type = SDL_QUIT;
                        SDL_PushEvent(&e);
                        break;
                    }
                    break;
                }
                case SDL_QUIT:
                    isRunning = false;
                    break;
                case SDL_KEYDOWN: 
                    cameraController.updateKey(e.key.keysym.scancode, true);
                    break;
                case SDL_KEYUP: 
                    cameraController.updateKey(e.key.keysym.scancode, false);
                    break;
                }
            }
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;
            frameTime = glm::min(frameTime, 0.5f);

            cameraController.update(frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
                
            float aspect = renderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1, 100);
            
            if (auto commandBuffer = renderer.beginFrame()) {
                int frameIndex = renderer.getFrameIndex();
                FrameInfo frameInfo{frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex]};
                
                //update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();
                
                //render
                renderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjs(frameInfo, gameObjs);
                renderer.endSwapChainRenderPass(commandBuffer);
                renderer.endFrame();
            }
        }
        vkDeviceWaitIdle(device.getDevice()); }
    
    void TriangleApp::loadGameObjs() {
        std::shared_ptr<Model> model = Model::createModelFromFile(device, "models/skull/skull.obj");

        auto triangle = GameObj::createGameObj();
        triangle.model = model;
        triangle.color = {0.5, 0.5, 0};
        triangle.transform.translation = {0.0f, 0.0f, 0.0f};
        triangle.transform.scale = {0.1f, 0.1f, 0.1f};

        gameObjs.push_back(std::move(triangle));
    }
}
