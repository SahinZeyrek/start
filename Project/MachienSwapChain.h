#pragma once

#include "MachienDevice.h"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace machien {

class MachienSwapChain {
 public:
  static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

  MachienSwapChain(MachienDevice &deviceRef, VkExtent2D extent);
  MachienSwapChain(MachienDevice& deviceRef, VkExtent2D extent, std::shared_ptr<MachienSwapChain> previous);

  ~MachienSwapChain();

  MachienSwapChain(const MachienSwapChain &) = delete;
  void operator=(const MachienSwapChain &) = delete;

  VkFramebuffer GetFrameBuffer(int index) { return m_pSwapChainFramebuffers[index]; }
  VkRenderPass GetRenderPass() { return m_pRenderPass; }
  VkImageView GetImageView(int index) { return m_pSwapChainImageViews[index]; }
  size_t ImageCount() { return m_pSwapChainImages.size(); }
  VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; }
  VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }
  uint32_t Width() { return m_SwapChainExtent.width; }
  uint32_t Height() { return m_SwapChainExtent.height; }

  float ExtentAspectRatio() {
    return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height);
  }
  VkFormat FindDepthFormat();

  VkResult AcquireNextImage(uint32_t *imageIndex);
  VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

  bool CompareSwapFormats(const MachienSwapChain& swapChain) const {
      return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat &&
             swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
  };
 private:
  void Init();
  void CreateSwapChain();
  void CreateImageViews();
  void CreateDepthResources();
  void CreateRenderPass();
  void CreateFramebuffers();
  void CreateSyncObjects();

  // Helper functions
  VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
      const std::vector<VkSurfaceFormatKHR> &availableFormats);
  VkPresentModeKHR ChooseSwapPresentMode(
      const std::vector<VkPresentModeKHR> &availablePresentModes);
  VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

  VkFormat m_SwapChainImageFormat;
  VkFormat m_SwapChainDepthFormat;
  VkExtent2D m_SwapChainExtent;

  std::vector<VkFramebuffer> m_pSwapChainFramebuffers;
  VkRenderPass m_pRenderPass;

  std::vector<VkImage> m_pDepthImages;
  std::vector<VkDeviceMemory> m_pDepthImageMemories;
  std::vector<VkImageView> m_pDepthImageViews;
  std::vector<VkImage> m_pSwapChainImages;
  std::vector<VkImageView> m_pSwapChainImageViews;

  MachienDevice& m_Device;
  VkExtent2D m_WindowExtent;

  VkSwapchainKHR m_pSwapChain;
  std::shared_ptr<MachienSwapChain> m_PrevSwapChain;

  std::vector<VkSemaphore> m_pImageAvailableSemaphores;
  std::vector<VkSemaphore> m_pRenderFinishedSemaphores;
  std::vector<VkFence> m_pInFlightFences;
  std::vector<VkFence> m_pImagesInFlight;
  size_t m_CurrentFrame = 0;
};

}  // namespace lve
