#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "MachienApp.h"
#include <iostream>
using namespace machien;
int main() 
{
	MachienApp app;
	try
	{
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
