#pragma once

#include "MachienObject.h"
#include "MachienWindow.h"
namespace machien
{
	class MachienKeyboard final
	{
	public:
		struct KeyMappings
		{
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_E;
			int moveDown = GLFW_KEY_Q;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
		};
		void MoveInPlaneXZ(GLFWwindow* window, float dt, MachienObject& object);
		void IncrementRenderMode(GLFWwindow* window,int& renderMode, int totalModes);
		KeyMappings keys{};
		float MoveSpeed{ 3.f };
		float LookSpeed{ 10.f };
	private:
		double m_LastMouseX{};
		double m_LastMouseY{};
		bool m_firstInput{true};
	};
}
