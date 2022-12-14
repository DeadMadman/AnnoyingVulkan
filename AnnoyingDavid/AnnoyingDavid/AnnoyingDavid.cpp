//refs
//https://github.com/blurrypiano/littleVulkanEngine
//https://vulkan-tutorial.com/
//https://learnopengl.com/

#include <iostream>

#include "TriangleApp.h"

int main(int argc, char* args[]) {
    svk::TriangleApp app{};
    try { app.run(); }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
