#pragma once

#include "MachienModel.h"
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
namespace machien
{
	struct TransformComponent2D
	{
		glm::vec3 Translation{};
		glm::vec3 Scale{ 1.f,1.f,1.f };
		glm::vec3 RadRotation{};
		glm::mat4 Mat4();
		glm::mat3 NormalMatrix();

	};
	class MachienObject2D final
	{
	public:
		using id = uint16_t;
		static MachienObject2D CreateObject()
		{
			static id currentId = 0;
			return MachienObject2D(currentId++);
		}
		id GetId() const { return m_Id; }

		MachienObject2D(const MachienObject2D&) = delete;
		MachienObject2D& operator=(const MachienObject2D&) = delete;
		MachienObject2D(MachienObject2D&&) = default;
		MachienObject2D& operator=(MachienObject2D&&) = default;

		std::shared_ptr<MachienModel> Model{};
		glm::vec3 Color{};
		TransformComponent2D Tranform{};

	private:
		MachienObject2D(id objId) : m_Id{ objId } {}
		id m_Id;
	};

}

