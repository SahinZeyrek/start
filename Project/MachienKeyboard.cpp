#include "MachienKeyboard.h"
namespace machien
{
	void MachienKeyboard::MoveInPlaneXZ(GLFWwindow* window, float dt, MachienObject& object)
	{
		glm::vec3 rotate{ 0 };

        const float sensitivity = 0.1f; // Adjust sensitivity as needed
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) 
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            double deltaX = mouseX - m_LastMouseX;
            double deltaY = mouseY - m_LastMouseY;

            m_LastMouseX = mouseX;
            m_LastMouseY = mouseY;


            rotate.y += static_cast<float>(deltaX) * sensitivity;
            rotate.x -= static_cast<float>(deltaY) * sensitivity;
        }
        if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon())
        {
            object.Tranform.RadRotation += LookSpeed * dt * glm::normalize(rotate);
        }
        object.Tranform.RadRotation.x = glm::clamp(object.Tranform.RadRotation.x, -1.5f, 1.5f);
        object.Tranform.RadRotation.y = glm::mod(object.Tranform.RadRotation.y, glm::two_pi<float>());

        float yaw = object.Tranform.RadRotation.y;
        const glm::vec3 forward{ sin(yaw),0.f,cos(yaw) };
        const glm::vec3 right{ forward.z,0.f,-forward.x };
        const glm::vec3 up{ 0.f,-1.f,0.f };

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS)  moveDir += forward;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forward;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS)   moveDir += right;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS)    moveDir -= right;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS)      moveDir += up;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS)    moveDir -= up;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon())
        {
            object.Tranform.Translation += MoveSpeed * dt * glm::normalize(moveDir);
        }


	}

}

