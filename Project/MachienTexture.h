#pragma once
#include "MachienDevice.h"
#include <string>
namespace machien
{
	class MachienTexture final
	{
	public:
		MachienTexture(MachienDevice& device);
		~MachienTexture();

		void CreateTextureImage(const std::string& texturePath);
		void CreateTextureImageView();
		void CreateTextureSampler();

		VkImageView GetTextureImageView() const;
		VkSampler GetTextureSampler() const;
		void Cleanup();

	private:
		MachienDevice& m_Device;
		VkImage m_Image;
		VkDeviceMemory m_ImageMemory;

		VkImageView m_ImageView;
		VkSampler m_Sampler;
	};

}

