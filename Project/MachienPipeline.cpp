#include "MachienPipeline.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>
namespace machien
{
	MachienPipeline::MachienPipeline(
		MachienDevice& device, 
		const std::string& vertFile, const std::string& fragFile, 
		const PipelineConfigInfo& configInfo,
		bool is3D) : m_Device(device)
	{
		CreatePipeline(vertFile, fragFile,configInfo,is3D);
	}
	MachienPipeline::~MachienPipeline()
	{
		vkDestroyShaderModule(m_Device.device(), m_pVertShaderModule, nullptr);
		vkDestroyShaderModule(m_Device.device(), m_pFragShaderModule, nullptr);

		vkDestroyPipeline(m_Device.device(), m_pGraphicsPipeline, nullptr);

	}
	
	std::vector<char> MachienPipeline::ReadFile(const std::string& file) {
		std::ifstream toRead{ file, std::ios::ate | std::ios::binary };

		if (!toRead.is_open())
		{
			throw std::runtime_error("failed to open file " + file);
		}

		size_t fileSize = static_cast<size_t>(toRead.tellg());
		std::vector<char> buffer(fileSize);

		toRead.seekg(0);
		toRead.read(buffer.data(), fileSize);

		toRead.close();
		return buffer;
	}
	void MachienPipeline::CreatePipeline(const std::string& vertFile, const std::string& fragFile, const PipelineConfigInfo& configInfo
										,bool is3D)
	{
		assert(configInfo.PipelineLayout != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no pipelineLayout provided in configInfo");

		assert(configInfo.RenderPass != VK_NULL_HANDLE &&
			"Cannot create graphics pipeline: no renderPass provided in configInfo");
		std::vector<char> vert = ReadFile(vertFile);
		std::vector<char> frag = ReadFile(fragFile);

		CreateShaderModule(vert, &m_pVertShaderModule);
		CreateShaderModule(frag, &m_pFragShaderModule);

		// ----------------------------------------------------
		// SHADER STAGE CREATE INFO
		// ----------------------------------------------------
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = m_pVertShaderModule;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;
		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = m_pFragShaderModule;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		// ----------------------------------------------------
		// VERTEX INPUT CREATE INFO
		// ----------------------------------------------------
		auto attributeDescriptions = MachienModel::Vertex::GetAttributeDescriptions(is3D);
		auto bindingDescriptions = MachienModel::Vertex::GetBindingDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());;
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		
		// ----------------------------------------------------
		// GRAPHICS PIPELINE CREATE INFO
		// ----------------------------------------------------

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.InputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.ViewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.RasterizationInfo;
		pipelineInfo.pMultisampleState = &configInfo.MultisampleInfo;
		pipelineInfo.pColorBlendState = &configInfo.ColorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.DepthStencilInfo;
		pipelineInfo.pDynamicState = &configInfo.DynamicStateInfo;

		pipelineInfo.layout = configInfo.PipelineLayout;
		pipelineInfo.renderPass = configInfo.RenderPass;
		pipelineInfo.subpass = configInfo.Subpass;

		if (vkCreateGraphicsPipelines(m_Device.device(), VK_NULL_HANDLE, 1, &pipelineInfo,
			nullptr, &m_pGraphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		}

	}
	void MachienPipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* module)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		if (vkCreateShaderModule(m_Device.device(), &createInfo, nullptr, module) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}


	};
	void MachienPipeline::DefaultPipelineConfigInfo(PipelineConfigInfo& info)
	{
		info.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		info.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		info.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
		
		info.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		info.ViewportInfo.viewportCount = 1;
		info.ViewportInfo.pViewports = nullptr;
		info.ViewportInfo.scissorCount = 1;
		info.ViewportInfo.pScissors = nullptr;

		

		info.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		info.RasterizationInfo.depthClampEnable = VK_FALSE;
		info.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		info.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
		info.RasterizationInfo.lineWidth = 1.0f;
		info.RasterizationInfo.cullMode = VK_CULL_MODE_NONE;
		info.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
		info.RasterizationInfo.depthBiasEnable = VK_FALSE;


		info.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		info.MultisampleInfo.sampleShadingEnable = VK_FALSE;
		info.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		info.MultisampleInfo.minSampleShading = 1.0f;           // Optional
		info.MultisampleInfo.pSampleMask = nullptr;             // Optional
		info.MultisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
		info.MultisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

		info.ColorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		info.ColorBlendAttachment.blendEnable = VK_FALSE;
		info.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		info.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		info.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		info.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		info.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		info.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		info.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		info.ColorBlendInfo.logicOpEnable = VK_FALSE;
		info.ColorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
		info.ColorBlendInfo.attachmentCount = 1;
		info.ColorBlendInfo.pAttachments = &info.ColorBlendAttachment;
		info.ColorBlendInfo.blendConstants[0] = 0.0f;  // Optional
		info.ColorBlendInfo.blendConstants[1] = 0.0f;  // Optional
		info.ColorBlendInfo.blendConstants[2] = 0.0f;  // Optional
		info.ColorBlendInfo.blendConstants[3] = 0.0f;  // Optional

		info.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		info.DepthStencilInfo.depthTestEnable = VK_TRUE;
		info.DepthStencilInfo.depthWriteEnable = VK_TRUE;
		info.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		info.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		info.DepthStencilInfo.minDepthBounds = 0.0f;  // Optional
		info.DepthStencilInfo.maxDepthBounds = 1.0f;  // Optional
		info.DepthStencilInfo.stencilTestEnable = VK_FALSE;
		info.DepthStencilInfo.front = {};  // Optional
		info.DepthStencilInfo.back = {};   // Optional

		info.DynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR };
		info.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		info.DynamicStateInfo.pDynamicStates = info.DynamicStateEnables.data();
		info.DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(info.DynamicStateEnables.size());
		info.DynamicStateInfo.flags = 0;
	
	}
	void MachienPipeline::Bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pGraphicsPipeline);

	}
}

