#pragma once

#include "MachienModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
namespace machien
{
	struct TransformComponent 
	{
		glm::vec3 Translation{};
		glm::vec3 Scale{1.f,1.f,1.f};
		glm::vec3 RadRotation{};
		glm::mat4 Mat4();
		glm::mat3 NormalMatrix();
		
	};
	class MachienObject final
	{
	public:
		using id = uint16_t;
		static MachienObject CreateObject()
		{
			static id currentId = 0;
			return MachienObject(currentId++);
		}
		id GetId() const { return m_Id; }

		MachienObject(const MachienObject&) = delete;
		MachienObject& operator=(const MachienObject&) = delete;
		MachienObject(MachienObject&&) = default;
		MachienObject& operator=(MachienObject&&) = default;

		std::shared_ptr<MachienModel> Model{};
		glm::vec3 Color{};
		TransformComponent Transform{};

	private:
		MachienObject(id objId) : m_Id{ objId } {}
		id m_Id;
	};

}

