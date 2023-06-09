#ifndef __VULKAN_SWAP_CHAIN_INC__
#define __VULkAN_SWAP_CHAIN_INC__

#include <memory>
#include <vector>
#include "vulkan/vulkan.h"

class VulkanInstance;
class VulkanDevice;

class VulkanSwapChain{
public:
  VulkanSwapChain(const std::shared_ptr<VulkanDevice> &dev);
  ~VulkanSwapChain();

  VkSurfaceKHR surface() { return _surface; };

  void set_surface(VkSurfaceKHR surface);

  void set_default_depth_format(VkFormat dep_format = VK_FORMAT_D24_UNORM_S8_UINT);

  void realize(uint32_t width, uint32_t height, bool vsync, bool fullscreen = false);

  uint32_t image_count() { return _images.size(); }

  struct DepthUnit {
    VkImage image;
    VkImageView view;
    VkDeviceMemory mem;
  };

  DepthUnit create_depth(uint32_t width, uint32_t height);

  VkRenderPass create_render_pass();

  std::vector<VkFramebuffer> create_frame_buffer(VkRenderPass vkPass, const DepthUnit &);

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