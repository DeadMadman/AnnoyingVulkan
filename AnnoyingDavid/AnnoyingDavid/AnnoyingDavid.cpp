// AnnoyingDavid.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include <iostream>
#include "TriangleApp.h"

int main(int argc, char* args[]) {
	
	TriangleApp app;
	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
