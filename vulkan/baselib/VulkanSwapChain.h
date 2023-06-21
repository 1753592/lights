#ifndef __VULKAN_SWAP_CHAIN_INC__
#define __VULkAN_SWAP_CHAIN_INC__

#include <memory>
#include <vector>
#include "vulkan/vulkan.h"
#include "VulkanDef.h"

class VulkanInstance;
class VulkanDevice;

class VulkanSwapChain{
public:
  VulkanSwapChain(const std::shared_ptr<VulkanDevice> &dev);
  ~VulkanSwapChain();

  VkSurfaceKHR surface() { return _surface; };

  void set_surface(VkSurfaceKHR surface);

  void set_default_depth_format(VkFormat dep_format = VK_FORMAT_D24_UNORM_S8_UINT);

  VkFormat color_format() { return _colorFormat; }

  VkFormat depth_format() { return _depthFormat; }

  void realize(uint32_t width, uint32_t height, bool vsync, bool fullscreen = false);

  uint32_t image_count() { return _images.size(); }

  ImageUnit create_depth_image(uint32_t width, uint32_t height);

  std::vector<VkFramebuffer> create_frame_buffer(VkRenderPass vkPass, const ImageUnit &);

  std::tuple<VkResult, uint32_t> acquire_image(VkSemaphore present_sema);

  VkResult queue_present(VkQueue queue, uint32_t index, VkSemaphore wait_sema = VK_NULL_HANDLE);

private:
  std::shared_ptr<VulkanDevice>    _device;

  VkSurfaceKHR _surface = VK_NULL_HANDLE;
  uint32_t _queueIndex = UINT32_MAX;
  VkFormat _colorFormat;
  VkColorSpaceKHR _colorSpace;
  VkFormat _depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
  VkSwapchainKHR _swapChain = VK_NULL_HANDLE;

  uint32_t _width = 0, _height = 0;

  std::vector<VkImage> _images;

  struct SwapChainBuffer{
    VkImage image;
    VkImageView view;
  };
  std::vector<SwapChainBuffer> _buffers;

  //PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR;
  //PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR;
  //PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
  //PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
  //PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR;
  //PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR;
  //PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR;
  //PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR;
  //PFN_vkQueuePresentKHR fpQueuePresentKHR;
};


#endif