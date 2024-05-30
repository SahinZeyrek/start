#include "MachienSwapChain.h"

// std
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>

namespace machien {

MachienSwapChain::MachienSwapChain(MachienDevice &deviceRef, VkExtent2D extent)
    : m_Device{deviceRef}, m_WindowExtent{extent} 
{
    Init();
}

MachienSwapChain::MachienSwapChain(MachienDevice& deviceRef, VkExtent2D extent, std::shared_ptr<MachienSwapChain> previous):
   m_Device{deviceRef},m_WindowExtent{ extent }, m_PrevSwapChain{previous}
{
    Init();
}

void MachienSwapChain::Init()
{
    CreateSwapChain();
    CreateImageViews();
    CreateRenderPass();
    CreateDepthResources();
    CreateFramebuffers();
    CreateSyncObjects();

}

MachienSwapChain::~MachienSwapChain() {
  for (auto imageView : m_pSwapChainImageViews) {
    vkDestroyImageView(m_Device.device(), imageView, nullptr);
  }
  m_pSwapChainImageViews.clear();

  if (m_pSwapChain != nullptr) {
    vkDestroySwapchainKHR(m_Device.device(), m_pSwapChain, nullptr);
    m_pSwapChain = nullptr;
  }

  for (int i = 0; i < m_pDepthImages.size(); i++) {
    vkDestroyImageView(m_Device.device(), m_pDepthImageViews[i], nullptr);
    vkDestroyImage(m_Device.device(), m_pDepthImages[i], nullptr);
    vkFreeMemory(m_Device.device(), m_pDepthImageMemories[i], nullptr);
  }

  for (auto framebuffer : m_pSwapChainFramebuffers) {
    vkDestroyFramebuffer(m_Device.device(), framebuffer, nullptr);
  }

  vkDestroyRenderPass(m_Device.device(), m_pRenderPass, nullptr);

  // cleanup synchronization objects
  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    vkDestroySemaphore(m_Device.device(), m_pRenderFinishedSemaphores[i], nullptr);
    vkDestroySemaphore(m_Device.device(), m_pImageAvailableSemaphores[i], nullptr);
    vkDestroyFence(m_Device.device(), m_pInFlightFences[i], nullptr);
  }
}

VkResult MachienSwapChain::AcquireNextImage(uint32_t *imageIndex) {
  vkWaitForFences(
      m_Device.device(),
      1,
      &m_pInFlightFences[m_CurrentFrame],
      VK_TRUE,
      std::numeric_limits<uint64_t>::max());

  VkResult result = vkAcquireNextImageKHR(
      m_Device.device(),
      m_pSwapChain,
      std::numeric_limits<uint64_t>::max(),
      m_pImageAvailableSemaphores[m_CurrentFrame],  // must be a not signaled semaphore
      VK_NULL_HANDLE,
      imageIndex);

  return result;
}

VkResult MachienSwapChain::SubmitCommandBuffers(
    const VkCommandBuffer *buffers, uint32_t *imageIndex) {
  if (m_pImagesInFlight[*imageIndex] != VK_NULL_HANDLE) {
    vkWaitForFences(m_Device.device(), 1, &m_pImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
  }
  m_pImagesInFlight[*imageIndex] = m_pInFlightFences[m_CurrentFrame];

  VkSubmitInfo submitInfo = {};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {m_pImageAvailableSemaphores[m_CurrentFrame]};
  VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;

  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = buffers;

  VkSemaphore signalSemaphores[] = {m_pRenderFinishedSemaphores[m_CurrentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  vkResetFences(m_Device.device(), 1, &m_pInFlightFences[m_CurrentFrame]);
  if (vkQueueSubmit(m_Device.graphicsQueue(), 1, &submitInfo, m_pInFlightFences[m_CurrentFrame]) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

  presentInfo.waitSemaphoreCount = 1;
  presentInfo.pWaitSemaphores = signalSemaphores;

  VkSwapchainKHR swapChains[] = {m_pSwapChain};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = swapChains;

  presentInfo.pImageIndices = imageIndex;

  auto result = vkQueuePresentKHR(m_Device.presentQueue(), &presentInfo);

  m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

  return result;
}

void MachienSwapChain::CreateSwapChain() {
  SwapChainSupportDetails swapChainSupport = m_Device.getSwapChainSupport();

  VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
  VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
  VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

  uint32_t ImageCount = swapChainSupport.capabilities.minImageCount + 1;
  if (swapChainSupport.capabilities.maxImageCount > 0 &&
      ImageCount > swapChainSupport.capabilities.maxImageCount) {
    ImageCount = swapChainSupport.capabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  createInfo.surface = m_Device.surface();

  createInfo.minImageCount = ImageCount;
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  QueueFamilyIndices indices = m_Device.findPhysicalQueueFamilies();
  uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

  if (indices.graphicsFamily != indices.presentFamily) {
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 2;
    createInfo.pQueueFamilyIndices = queueFamilyIndices;
  } else {
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;      // Optional
    createInfo.pQueueFamilyIndices = nullptr;  // Optional
  }

  createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  createInfo.oldSwapchain = (m_PrevSwapChain == nullptr ? VK_NULL_HANDLE : m_PrevSwapChain->m_pSwapChain);

  if (vkCreateSwapchainKHR(m_Device.device(), &createInfo, nullptr, &m_pSwapChain) != VK_SUCCESS) {
    throw std::runtime_error("failed to create swap chain!");
  }

  // we only specified a minimum number of images in the swap chain, so the implementation is
  // allowed to create a swap chain with more. That's why we'll first query the final number of
  // images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
  // retrieve the handles.
  vkGetSwapchainImagesKHR(m_Device.device(), m_pSwapChain, &ImageCount, nullptr);
  m_pSwapChainImages.resize(ImageCount);
  vkGetSwapchainImagesKHR(m_Device.device(), m_pSwapChain, &ImageCount, m_pSwapChainImages.data());

  m_SwapChainImageFormat = surfaceFormat.format;
  m_SwapChainExtent = extent;
}

void MachienSwapChain::CreateImageViews() {
  m_pSwapChainImageViews.resize(m_pSwapChainImages.size());
  // could simply this with m_Devic.device().CreateImageView(m_pSwapChainImages[i],swapChainImageFormat);
  for (size_t i = 0; i < m_pSwapChainImages.size(); i++) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_pSwapChainImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = m_SwapChainImageFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_pSwapChainImageViews[i]) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void MachienSwapChain::CreateRenderPass() {
  VkAttachmentDescription depthAttachment{};
  depthAttachment.format = FindDepthFormat();
  depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depthAttachmentRef{};
  depthAttachmentRef.attachment = 1;
  depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format = GetSwapChainImageFormat();
  colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment = 0;
  colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorAttachmentRef;
  subpass.pDepthStencilAttachment = &depthAttachmentRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcAccessMask = 0;
  dependency.srcStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstSubpass = 0;
  dependency.dstStageMask =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.dstAccessMask =
      VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  VkRenderPassCreateInfo renderPassInfo = {};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments = attachments.data();
  renderPassInfo.subpassCount = 1;
  renderPassInfo.pSubpasses = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies = &dependency;

  if (vkCreateRenderPass(m_Device.device(), &renderPassInfo, nullptr, &m_pRenderPass) != VK_SUCCESS) {
    throw std::runtime_error("failed to create render pass!");
  }
}

void MachienSwapChain::CreateFramebuffers() {
  m_pSwapChainFramebuffers.resize(ImageCount());
  for (size_t i = 0; i < ImageCount(); i++) {
    std::array<VkImageView, 2> attachments = {m_pSwapChainImageViews[i], m_pDepthImageViews[i]};

    VkExtent2D swapChainExtent = GetSwapChainExtent();
    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_pRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(
        m_Device.device(),
            &framebufferInfo,
            nullptr,
            &m_pSwapChainFramebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void MachienSwapChain::CreateDepthResources() {
  VkFormat depthFormat = FindDepthFormat();
  m_SwapChainDepthFormat = depthFormat;
  VkExtent2D swapChainExtent = GetSwapChainExtent();

  m_pDepthImages.resize(ImageCount());
  m_pDepthImageMemories.resize(ImageCount());
  m_pDepthImageViews.resize(ImageCount());

  for (int i = 0; i < m_pDepthImages.size(); i++) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = swapChainExtent.width;
    imageInfo.extent.height = swapChainExtent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = depthFormat;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.flags = 0;

    m_Device.createImageWithInfo(
        imageInfo,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_pDepthImages[i],
        m_pDepthImageMemories[i]);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_pDepthImages[i];
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = depthFormat;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_Device.device(), &viewInfo, nullptr, &m_pDepthImageViews[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create texture image view!");
    }
  }
}

void MachienSwapChain::CreateSyncObjects() {
  m_pImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_pRenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  m_pInFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
  m_pImagesInFlight.resize(ImageCount(), VK_NULL_HANDLE);

  VkSemaphoreCreateInfo semaphoreInfo = {};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo = {};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(m_Device.device(), &semaphoreInfo, nullptr, &m_pImageAvailableSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateSemaphore(m_Device.device(), &semaphoreInfo, nullptr, &m_pRenderFinishedSemaphores[i]) !=
            VK_SUCCESS ||
        vkCreateFence(m_Device.device(), &fenceInfo, nullptr, &m_pInFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create synchronization objects for a frame!");
    }
  }
}

VkSurfaceFormatKHR MachienSwapChain::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR> &availableFormats) {
  for (const auto &availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return availableFormats[0];
}

VkPresentModeKHR MachienSwapChain::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR> &availablePresentModes) {
  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      std::cout << "Present mode: Mailbox" << std::endl;
      return availablePresentMode;
    }
  }

  for (const auto &availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
      std::cout << "Present mode: Immediate" << std::endl;
      return availablePresentMode;
    }
  }

  std::cout << "Present mode: V-Sync" << std::endl;
  return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D MachienSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent = m_WindowExtent;
    actualExtent.width = std::max(
        capabilities.minImageExtent.width,
        std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(
        capabilities.minImageExtent.height,
        std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
  }
}

VkFormat MachienSwapChain::FindDepthFormat() {
  return m_Device.findSupportedFormat(
      {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
      VK_IMAGE_TILING_OPTIMAL,
      VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

}  // namespace lve
