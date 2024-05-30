#include "MachienTexture.h"
#include "vulkan/vulkan.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>
namespace machien
{
	MachienTexture::MachienTexture(MachienDevice& device) : m_Device{device}
	{
		
	}

	MachienTexture::~MachienTexture()
	{
		Cleanup();
	}

	void MachienTexture::CreateTextureImage(const std::string& texturePath)
	{
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}
		// temporary Staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		m_Device.createBuffer(imageSize,VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
		void* data;
		vkMapMemory(m_Device.device(), stagingBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, pixels, static_cast<size_t>(imageSize));
		vkUnmapMemory(m_Device.device(), stagingBufferMemory);
		stbi_image_free(pixels);

		uint32_t arrayLayers = 1, mipLevels = 1;

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = static_cast<uint32_t>(texWidth);
		imageInfo.extent.height = static_cast<uint32_t>(texHeight);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = mipLevels;
		imageInfo.arrayLayers = arrayLayers;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0; // Optional

		uint32_t layerCount = 1;

		m_Device.TransitionImageLayout(m_Image, 
			VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels, layerCount);
		m_Device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_Image, m_ImageMemory);

		// transition image layout for fragment shader
		m_Device.TransitionImageLayout(m_Image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels, layerCount);
		
		// cleanup
		vkDestroyBuffer(m_Device.device(), stagingBuffer, nullptr);
		vkFreeMemory(m_Device.device(), stagingBufferMemory, nullptr);
	}

	void MachienTexture::CreateTextureImageView()
	{
		uint32_t mipLevels = 1;
		m_ImageView = m_Device.CreateImageView(m_Image, VK_FORMAT_R8G8B8A8_SRGB);
	}

	void MachienTexture::CreateTextureSampler()
	{
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;

		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.anisotropyEnable = VK_TRUE;

		m_Device.SetPhyiscalDeviceProperties(m_Device.GetPhysicalDevice(), m_Device.properties); // get using device class

		samplerInfo.maxAnisotropy = m_Device.properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS; // comparison function
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // mipmap interpolation
		samplerInfo.minLod = 0; // minimum level of detail
		samplerInfo.maxLod = 0; // maximum level of detail
		samplerInfo.mipLodBias = 0; // level of detail bias

		if (vkCreateSampler(m_Device.device(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create texture sampler!");
		}
	}


	VkImageView MachienTexture::GetTextureImageView() const
	{
		return VkImageView();
	}

	VkSampler MachienTexture::GetTextureSampler() const
	{
		return VkSampler();
	}

	void MachienTexture::Cleanup()
	{
		if (m_Sampler != VK_NULL_HANDLE) {
			vkDestroySampler(m_Device.device(), m_Sampler, nullptr);
			m_Sampler = VK_NULL_HANDLE;
		}
		if (m_ImageView != VK_NULL_HANDLE) {
			vkDestroyImageView(m_Device.device(), m_ImageView, nullptr);
			m_ImageView = VK_NULL_HANDLE;
		}
		if (m_Image != VK_NULL_HANDLE) {
			vkDestroyImage(m_Device.device(), m_Image, nullptr);
			m_Image = VK_NULL_HANDLE;
		}
		if (m_ImageMemory != VK_NULL_HANDLE) {
			vkFreeMemory(m_Device.device(), m_ImageMemory, nullptr);
			m_ImageMemory = VK_NULL_HANDLE;
		}
	}


}
