#include "MachienModel.h"
#include <cassert>
#include <string>
#include "MachienUtils.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

namespace std {
	template<>
	struct hash<machien::MachienModel::Vertex>
	{
		size_t operator()(machien::MachienModel::Vertex const& vertex) const
		{
			size_t seed = 0;
			machien::HashCombine(seed, vertex.Position, vertex.Color, vertex.Normal, vertex.UV);
			return seed;
		}
	};
}

namespace machien
{

	MachienModel::MachienModel(MachienDevice& device, const MachienModel::Builder& builder, bool is3D) : m_Device{device},
		m_Is3D{is3D}
		
	{
		CreateVertexBuffers(builder.vertices);
		CreateIndexBuffers(builder.indices);
	}

	MachienModel::~MachienModel()
	{
		
	}

	void MachienModel::Bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer buffer[] = { m_VertexBuffer->getBuffer()};
		VkDeviceSize offset[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffer, offset);
		if (m_HasIndexBuffer)
		{
			vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
		}
	}

	void MachienModel::Draw(VkCommandBuffer commandBuffer)
	{
		if (m_HasIndexBuffer)
		{
			vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
		}
		else 
		{
			vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
		}

	}

	void MachienModel::CreateVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_VertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_VertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
		uint32_t vertexSize = sizeof(vertices[0]);

		MachienBuffer stagingBuffer
		{
			m_Device,vertexSize,m_VertexCount,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)vertices.data());

		m_VertexBuffer = std::make_unique<MachienBuffer>(
			m_Device, vertexSize, m_VertexCount,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
			);

		m_Device.copyBuffer(stagingBuffer.getBuffer(), m_VertexBuffer->getBuffer(), bufferSize);

	}
	void MachienModel::CreateIndexBuffers(const std::vector<uint32_t>& indices)
	{
		m_IndexCount = static_cast<uint32_t>(indices.size());
		m_HasIndexBuffer = m_IndexCount > 0;
		if (!m_HasIndexBuffer) { return; }

		VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
		uint32_t indexSize = sizeof(indices[0]);
		MachienBuffer stagingBuffer
		{
			m_Device,indexSize,m_IndexCount,VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)indices.data());


		m_IndexBuffer = std::make_unique<MachienBuffer>(
			m_Device, indexSize, m_IndexCount,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);

		m_Device.copyBuffer(stagingBuffer.getBuffer(), m_IndexBuffer->getBuffer(), bufferSize);

	}

	std::unique_ptr<MachienModel> MachienModel::CreateModelFromFile(MachienDevice& device, const std::string& filePath)
	{
		Builder builder{};
		builder.LoadModel(filePath);
		return std::make_unique<MachienModel>(device, builder);
	}

	std::unique_ptr<MachienModel> MachienModel::CreateSquare(MachienDevice& device,glm::vec2 center, float width, float height)
	{
		Builder builder;
		std::vector<Vertex> vertices
		{
			{{center.x -width / 2.f,center.y -height / 2.f,1.f}},
			{{center.x +width / 2.f,center.y -height / 2.f,1.f}},
			{{center.x + width / 2.f,center.y +height / 2.f,1.f}},
			{{center.x -width / 2.f,center.y +height / 2.f,1.f}},
		};
		std::vector<uint32_t> indices = {
	   0, 1, 2,  // First triangle (bottom-right triangle)
	   2, 3, 0   // Second triangle (top-left triangle)
		};
		builder.vertices = vertices;
		builder.indices = indices;
		return std::make_unique<MachienModel>(device, builder, false);
	}

	std::unique_ptr<MachienModel> MachienModel::CreateOval(MachienDevice& device, float radiusX, float radiusY, int numSegments)
	{
		Builder builder{};

		std::vector<Vertex> vertices;

		// Add center vertex
		vertices.push_back({ {0.0f, 0.0f,1.f} });

		// Calculate vertices for the oval
		for (int i = 0; i <= numSegments; ++i)
		{
			float angle = static_cast<float>(i) / static_cast<float>(numSegments) * glm::two_pi<float>();
			float x = radiusX * cosf(angle);
			float y = radiusY * sinf(angle);
			vertices.push_back({ {x, y,1.f} });
		}
		builder.vertices = vertices;

		return std::make_unique<MachienModel>(device,builder,false);

	}

	void MachienModel::Builder::LoadModel(const std::string& filePath)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn,err;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials,&warn, &err, filePath.c_str()))
		{
			throw std::runtime_error(err);
		}
		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, uint32_t> uniqueVertices{};

		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				Vertex vertex{};
				if (index.vertex_index >= 0)
				{
					vertex.Position = { attrib.vertices[3 * index.vertex_index + 0],
										attrib.vertices[3 * index.vertex_index + 1] ,
										attrib.vertices[3 * index.vertex_index + 2] };
				}
				if (index.vertex_index >= 0)
				{
					vertex.Color = {	attrib.colors[3 * index.vertex_index + 0],
										attrib.colors[3 * index.vertex_index + 1] ,
										attrib.colors[3 * index.vertex_index + 2] };
				}
				if (index.normal_index >= 0)
				{
					vertex.Normal = {	attrib.normals[3 * index.normal_index + 0],
										attrib.normals[3 * index.normal_index + 1] ,
										attrib.normals[3 * index.normal_index + 2] };
				}
				if (index.texcoord_index >= 0)
				{
					vertex.UV = {		attrib.texcoords[2 * index.texcoord_index + 0],
										attrib.texcoords[2 * index.texcoord_index + 1] };
				}
				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}
				indices.push_back(uniqueVertices[vertex]);
			}
		}
	}


	std::vector<VkVertexInputBindingDescription> MachienModel::Vertex::GetBindingDescriptions()
	{
		return std::vector<VkVertexInputBindingDescription> 
		{
			VkVertexInputBindingDescription
			{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
			}
		};
		
	}

	std::vector<VkVertexInputAttributeDescription> MachienModel::Vertex::GetAttributeDescriptions(bool is3D)
	{
		if (is3D == true)
		{
			return std::vector<VkVertexInputAttributeDescription>
			{
				VkVertexInputAttributeDescription
				{
					.location = 0,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex,Position)
				},
					VkVertexInputAttributeDescription
				{
					.location = 1,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex,Color)
				},
					VkVertexInputAttributeDescription
				{
					.location = 2,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex,Normal)
				},
					VkVertexInputAttributeDescription
				{
					.location = 3,
					.binding = 0,
					.format = VK_FORMAT_R32G32_SFLOAT,
					.offset = offsetof(Vertex,UV)
				}

			};
		}
		else
		{
			return std::vector<VkVertexInputAttributeDescription>
			{
				VkVertexInputAttributeDescription
				{
					.location = 0,
					.binding = 0,
					.format = VK_FORMAT_R32G32_SFLOAT,
					.offset = offsetof(Vertex,Position)
				},
					VkVertexInputAttributeDescription
				{
					.location = 1,
					.binding = 0,
					.format = VK_FORMAT_R32G32B32_SFLOAT,
					.offset = offsetof(Vertex,Color)
				}

			};
		}
		
	}


}

