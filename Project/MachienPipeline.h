#pragma once
#include "MachienDevice.h"
#include "MachienModel.h"

#include <string>
#include <vector>
namespace machien 
{
	struct PipelineConfigInfo 
	{
		PipelineConfigInfo() = default;

		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo(PipelineConfigInfo&&) = delete;
		PipelineConfigInfo operator=(PipelineConfigInfo&&) = delete;

		VkPipelineViewportStateCreateInfo ViewportInfo;
		VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo RasterizationInfo;
		VkPipelineMultisampleStateCreateInfo MultisampleInfo;
		VkPipelineColorBlendAttachmentState ColorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo ColorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo DepthStencilInfo;
		std::vector<VkDynamicState> DynamicStateEnables;
		VkPipelineDynamicStateCreateInfo DynamicStateInfo;
		VkPipelineLayout PipelineLayout = nullptr;
		VkRenderPass RenderPass = nullptr;
		uint32_t Subpass = 0;
	};
	class MachienPipeline final
	{
	public:
		MachienPipeline( MachienDevice& device, 
			const std::string& vertFile,
			const std::string& fragFile, const PipelineConfigInfo& configInfo,
			bool is3D = true);
		~MachienPipeline();

		static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
		void Bind(VkCommandBuffer commandBuffer);

		MachienPipeline(const MachienPipeline&) = delete;
		MachienPipeline(MachienPipeline&&) = delete;
		MachienPipeline operator=(const MachienPipeline&) = delete;
		MachienPipeline operator=(MachienPipeline&&) = delete;
	private:
		static std::vector<char> ReadFile(const std::string& file);

		void CreatePipeline(const std::string& vertFile, const std::string& fragFile, const PipelineConfigInfo& configInfo
			, bool is3D);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* module);

		MachienDevice& m_Device;
		VkPipeline m_pGraphicsPipeline;
		VkShaderModule m_pVertShaderModule;
		VkShaderModule m_pFragShaderModule;

	};
}

