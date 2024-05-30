#include "MachienWindow.h"
#include <stdexcept>
namespace machien
{
	MachienWindow::MachienWindow(uint32_t Width, uint32_t Height, const std::string& name) :
		m_Width{Width},m_Height{Height},m_WindowName{name}
	{
		InitWindow();
	}

	MachienWindow::~MachienWindow()
	{
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	void MachienWindow::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
	{
		if (glfwCreateWindowSurface(instance, m_pWindow, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface ");
		}
	}

	void MachienWindow::OnResize(GLFWwindow* userWindow, int width, int height)
	{
		auto window = reinterpret_cast<MachienWindow*>(glfwGetWindowUserPointer(userWindow));
		window->m_FrameBufferResized = true;
		window->m_Width = width;
		window->m_Height = height;

	}

	void MachienWindow::InitWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		m_pWindow = glfwCreateWindow(m_Width, m_Height, m_WindowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_pWindow, this);
		glfwSetFramebufferSizeCallback(m_pWindow, OnResize);
	}

}
