#include "MachienRenderSystem.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <iterator>
#include <iostream>

namespace machien
{
	struct PushConstantData
	{
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 NormalMatrix{ 1.f };
	};

	MachienRenderSystem::MachienRenderSystem(MachienDevice& device,VkRenderPass renderPass, VkDescriptorSetLayout DescriptorSetLayout) :
		m_Device{device}
	{
		CreatePipelineLayout(DescriptorSetLayout);
		CreatePipeline(renderPass);

	}
	MachienRenderSystem::~MachienRenderSystem()
	{
		vkDestroyPipelineLayout(m_Device.device(), m_pPipelineLayout, nullptr);

	}
	
	

	void MachienRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout DescriptorSetLayout)
	{
		VkPushConstantRange pushConstantRange;
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ DescriptorSetLayout };


		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(m_Device.device(), &pipelineLayoutInfo, nullptr, &m_pPipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline Error");
		}
	}

	void MachienRenderSystem::CreatePipeline(VkRenderPass renderPass)
	{
		assert(m_pPipelineLayout != nullptr && "Cannot Create pipeline before pipeline layout");
		PipelineConfigInfo pipelineConfig{};
		MachienPipeline::DefaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.RenderPass = renderPass;
		pipelineConfig.PipelineLayout = m_pPipelineLayout;
		m_Pipeline = std::make_unique<MachienPipeline>(m_Device, "shaders/shader.vert.spv",
			"shaders/shader.frag.spv", pipelineConfig,true);
		//m_2DPipeline = std::make_unique<MachienPipeline>(m_Device, "shaders/2dShader.vert.spv",
		//	"shaders/2dShader.frag.spv", pipelineConfig,false);
	}


	void MachienRenderSystem::DrawObjects(FrameInfo& frameInfo, std::vector<MachienObject>& objects)
	{
		m_Pipeline->Bind(frameInfo.CommandBuffer);
		//auto projectionView = frameInfo.Camera.GetProjection() * frameInfo.Camera.GetView();

		vkCmdBindDescriptorSets(
			frameInfo.CommandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			m_pPipelineLayout, 0, 1,
			&frameInfo.DescriptorSet,
			0, nullptr);
		for (auto& obj : objects)
		{
			PushConstantData data{};

			data.modelMatrix = obj.Transform.Mat4();
			data.NormalMatrix = obj.Transform.NormalMatrix();


			vkCmdPushConstants(frameInfo.CommandBuffer, m_pPipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &data);
			obj.Model->Bind(frameInfo.CommandBuffer);
			obj.Model->Draw(frameInfo.CommandBuffer);
		}
		//m_2DPipeline->Bind(commandBuffer);
		//
		//for (auto& obj2D : objects2D)
		//{
		//	PushConstantData data{};
		//	auto modelMatrix = obj2D.Tranform.Mat4();
		//
		//	data.Transform = projectionView * modelMatrix;
		//	data.NormalMatrix = obj2D.Tranform.NormalMatrix();
		//	vkCmdPushConstants(commandBuffer, m_pPipelineLayout,
		//		VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &data);
		//	obj2D.Model->Bind(commandBuffer);
		//	obj2D.Model->Draw(commandBuffer);
		//}
	}


}
