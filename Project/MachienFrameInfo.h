#pragma once

#include "MachienCamera.h"
#include <vulkan/vulkan.h>

namespace machien
{
	struct FrameInfo
	{
		int FrameIndex;
		float FrameTime;
		VkCommandBuffer CommandBuffer;
		MachienCamera& Camera;
		VkDescriptorSet DescriptorSet;
	};
}