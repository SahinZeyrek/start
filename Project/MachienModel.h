#pragma once
#include "MachienDevice.h"
#include "MachienBuffer.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <tiny_obj_loader.h>


namespace machien
{
	class MachienModel final
	{
	public:
		struct Vertex
		{
			glm::vec3 Position{};
			glm::vec3 Color{};
			glm::vec3 Normal{};
			glm::vec2 UV{};
			glm::vec3 Tangent{};

			static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(bool is3D);


			bool operator==(const Vertex& other) const
			{
				return Position == other.Position &&
					   Normal == other.Normal &&
					   UV == other.UV &&
					   Tangent == other.Tangent;
			}
		};
		struct Builder
		{
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void LoadModel(const std::string& filePath);
		};
		MachienModel(MachienDevice& device,const MachienModel::Builder& builder, bool is3D = true);
		~MachienModel();

		MachienModel(const MachienModel&) = delete;
		MachienModel(MachienModel&&) = delete;
		MachienModel operator=(const MachienModel&) = delete;
		MachienModel operator=(MachienModel&&) = delete;
		
		static std::unique_ptr<MachienModel> CreateModelFromFile(MachienDevice& device, const std::string& filePath);
		static std::unique_ptr<MachienModel> CreateCube(MachienDevice& device);
		static std::unique_ptr<MachienModel> CreateSphere(MachienDevice& device);
		static std::unique_ptr<MachienModel> CreateSquare(MachienDevice& device, glm::vec2 center , float width, float height);
		static std::unique_ptr<MachienModel> CreateOval(MachienDevice& device, float radiusX, float radiusY, int numSegments);
		void Bind(VkCommandBuffer commandBuffer);
		void Draw(VkCommandBuffer commandBuffer);

	private:
		void CreateVertexBuffers(const std::vector<Vertex>& vertices);
		void CreateIndexBuffers(const std::vector<uint32_t>& indices);
		static glm::vec3 CalculateTangent(const glm::vec3& pos0, const glm::vec3& pos1, const glm::vec3& pos2,
								   const glm::vec2& uv0, const glm::vec2& uv1, const glm::vec2& uv2);
		static glm::vec3 GetPosition(const tinyobj::attrib_t& attrib, int index);
		static glm::vec3 GetNormal(const tinyobj::attrib_t& attrib, int index);
		static glm::vec2 GetTexCoord(const tinyobj::attrib_t& attrib, int index);
		MachienDevice& m_Device;
		std::unique_ptr<MachienBuffer> m_VertexBuffer;
		uint32_t m_VertexCount;

		bool m_HasIndexBuffer = false;
		std::unique_ptr<MachienBuffer> m_IndexBuffer;
		uint32_t m_IndexCount;

		bool m_Is3D;
	};

}

