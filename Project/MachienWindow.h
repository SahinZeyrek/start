#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
namespace machien
{
	class MachienWindow final
	{
	public:
		MachienWindow(uint32_t Width, uint32_t Height, const std::string& name);
		~MachienWindow();

		MachienWindow(const MachienWindow&) = delete;
		MachienWindow(MachienWindow&&) = delete;
		MachienWindow operator=(const MachienWindow&) = delete;
		MachienWindow operator=(MachienWindow&&) = delete;

		VkExtent2D GetExtent() { return { static_cast<uint32_t>(m_Width),static_cast<uint32_t>(m_Height) }; }

		bool IsClosed() { return glfwWindowShouldClose(m_pWindow); }
		bool WasWindowResized() { return m_FrameBufferResized; }
		void ResetWindowResizedFlag() { m_FrameBufferResized = false; }
		void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
		GLFWwindow* GetGLFWWindow() const { return m_pWindow; }

	private:
		static void OnResize(GLFWwindow* window, int width, int height);
		GLFWwindow* m_pWindow;
		void InitWindow();
		uint32_t m_Width, m_Height;
		bool m_FrameBufferResized = false;
		std::string m_WindowName;
	};
}