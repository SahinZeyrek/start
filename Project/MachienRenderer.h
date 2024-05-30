#pragma once
#include "MachienWindow.h"
#include "MachienDevice.h"
#include "MachienSwapChain.h"
#include <cassert>
#include <memory>
#include <vector>

namespace machien
{
	class MachienRenderer final
	{
	public:
		MachienRenderer(MachienWindow& window, MachienDevice& device);
		~MachienRenderer();


		MachienRenderer(const MachienRenderer&) = delete;
		MachienRenderer(MachienRenderer&&) = delete;
		MachienRenderer& operator=(const MachienRenderer&) = delete;
		MachienRenderer& operator=(MachienRenderer&&) = delete;

		VkRenderPass GetSwapChainRenderPass() const { return m_Swapchain->GetRenderPass(); }
		float GetAspectRatio() const { return m_Swapchain->ExtentAspectRatio(); }
		bool IsFrameInProgres() const { return m_IsFrameStarted; }
		VkCommandBuffer GetCurrentCommandBuffer() const
		{
			assert(m_IsFrameStarted && "Cannot get command buffer when frame not in progress");
			return m_CommandBuffers[m_CurrentFrameIndex];
		}
		int GetFrameIndex() const
		{
			assert(m_IsFrameStarted && "Cannot get frame index when frame not in progress");
			return m_CurrentFrameIndex;
		}

		VkCommandBuffer BeginFrame();
		void EndFrame();
		void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		void CreateCommandBuffers();
		void FreeCommandBuffers();
		void RecreateSwapChain();

		MachienWindow& m_Window;
		MachienDevice& m_Device;
		std::unique_ptr<MachienSwapChain> m_Swapchain;
		std::vector<VkCommandBuffer> m_CommandBuffers;
		
		uint32_t m_CurrentImageIndex;
		int m_CurrentFrameIndex{ 0 };
		bool m_IsFrameStarted = false;
	};

}
