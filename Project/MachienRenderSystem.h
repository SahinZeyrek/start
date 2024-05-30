#pragma once
#include "MachienPipeline.h"
#include "MachienDevice.h"
#include "MachienObject.h"
#include "MachienObject2D.h"

#include "MachienCamera.h"
#include "MachienFrameInfo.h"
#include <memory>
#include <vector>

namespace machien
{
	class MachienRenderSystem final
	{
	public:
		MachienRenderSystem(MachienDevice& device,VkRenderPass renderPass,VkDescriptorSetLayout DescriptorSetLayout);
		~MachienRenderSystem();


		MachienRenderSystem(const MachienRenderSystem&) = delete;
		MachienRenderSystem(MachienRenderSystem&&) = delete;
		MachienRenderSystem& operator=(const MachienRenderSystem&) = delete;
		MachienRenderSystem& operator=(MachienRenderSystem&&) = delete;

		void DrawObjects(FrameInfo& frameInfo , std::vector<MachienObject>& objects);

	private:
		void CreatePipelineLayout(VkDescriptorSetLayout DescriptorSetLayout);
		void CreatePipeline(VkRenderPass renderPass);

		MachienDevice& m_Device;
		std::unique_ptr<MachienPipeline> m_Pipeline;
		std::unique_ptr<MachienPipeline> m_2DPipeline;
		VkPipelineLayout m_pPipelineLayout;

	};

}
