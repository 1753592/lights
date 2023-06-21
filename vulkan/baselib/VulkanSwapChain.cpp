#include "VulkanSwapChain.h"

#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "VulkanTools.h"

#include <array>

VulkanSwapChain::VulkanSwapChain(const std::shared_ptr<VulkanDevice> &dev)
  : _device(dev)
{
  //fpGetPhysicalDeviceSurfaceSupportKHR =
  //    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
  //fpGetPhysicalDeviceSurfaceCapabilitiesKHR =
  //    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
  //fpGetPhysicalDeviceSurfaceFormatsKHR =
  //    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
  //fpGetPhysicalDeviceSurfacePresentModesKHR =
  //    reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

  //fpCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(*_device, "vkCreateSwapchainKHR"));
  //fpDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(*_device, "vkDestroySwapchainKHR"));
  //fpGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(*_device, "vkGetSwapchainImagesKHR"));
  //fpAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(*_device, "vkAcquireNextImageKHR"));
  //fpQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(*_device, "vkQueuePresentKHR"));
}

VulkanSwapChain::~VulkanSwapChain() 
{
  if (_swapChain != VK_NULL_HANDLE) {
    for (uint32_t i = 0; i < _buffers.size(); i++) {
      vkDestroyImageView(*_device, _buffers[i].view, nullptr);
    }
  }

  if(_swapChain)
    vkDestroySwapchainKHR(*_device, _swapChain, nullptr);
  _swapChain = VK_NULL_HANDLE;

  _surface = VK_NULL_HANDLE;
}

void VulkanSwapChain::set_surface(VkSurfaceKHR surface) 
{
  _surface = surface;

  auto &props = _device->queue_family_properties();
  std::vector<VkBool32> supportsPresent(props.size());
  for (int i = 0; i < props.size(); i++)
    vkGetPhysicalDeviceSurfaceSupportKHR(_device->physical_device(), i, surface, &supportsPresent[i]);

  uint32_t graphicIndex = UINT32_MAX;
  uint32_t presentIndex = UINT32_MAX;
  for(int i = 0; i < supportsPresent.size(); i++) {
    if(props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      if (graphicIndex == UINT32_MAX)
        graphicIndex = i;
      if(supportsPresent[i]) {
        graphicIndex = i;
        presentIndex = i;
        break;
      }
    }
  }
  if(presentIndex == UINT32_MAX) {
    for(int i = 0; i < supportsPresent.size(); i++) {
      if (supportsPresent[i]) {
        presentIndex = i;
        break;
      }
    }
  }

  if (graphicIndex == UINT32_MAX || presentIndex == UINT32_MAX)
    throw std::runtime_error("Could not find a graphic or present queue!");

  if (graphicIndex != presentIndex)
    throw std::runtime_error("Separate graphic and present queue are not supported yet !");

  _queueIndex = graphicIndex;

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(_device->physical_device(), surface, &formatCount, 0);
  std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
  vkGetPhysicalDeviceSurfaceFormatsKHR(_device->physical_device(), surface, &formatCount, surfaceFormats.data());

  if(formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED) {
    _colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
  }else{
    _colorFormat = VK_FORMAT_UNDEFINED;
    for(auto &surfaceFormat : surfaceFormats) {
      if(surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM) {
        _colorFormat = surfaceFormat.format;
        _colorSpace = surfaceFormat.colorSpace;
        break;
      }
    }
    if(_colorFormat == VK_FORMAT_UNDEFINED) {
      _colorFormat = surfaceFormats[0].format;
      _colorSpace = surfaceFormats[0].colorSpace;
    }
  }
}

void VulkanSwapChain::set_default_depth_format(VkFormat dep_format) 
{
  _depthFormat = dep_format;
}

void VulkanSwapChain::realize(uint32_t width, uint32_t height, bool vsync, bool fullscreen) 
{
  _width = width; _height = height;
  vkDeviceWaitIdle(*_device);

  VkSwapchainKHR oldchain = _swapChain;

  VkSurfaceCapabilitiesKHR surfaceCaps;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device->physical_device(), _surface, &surfaceCaps);

  uint32_t presetModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(_device->physical_device(), _surface, &presetModeCount, nullptr);
  std::vector<VkPresentModeKHR> presentModes(presetModeCount);
  vkGetPhysicalDeviceSurfacePresentModesKHR(_device->physical_device(), _surface, &presetModeCount, presentModes.data());

  VkExtent2D chainExtent = {};
  chainExtent.width = width;
  chainExtent.height = height;

  VkPresentModeKHR chainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

  if(!vsync) {
    for(int i = 0; i < presetModeCount; i++) {
      if(presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
        chainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
        break;
      }
      if (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR) {
        chainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
      }
    }
  }

  uint32_t desireImages = surfaceCaps.minImageCount + 1;
  if (surfaceCaps.maxImageCount > 0 && desireImages > surfaceCaps.maxImageCount)
    desireImages = surfaceCaps.maxImageCount;

  VkSurfaceTransformFlagsKHR preTransform;
  if (surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    // We prefer a non-rotated transform
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  } else {
    preTransform = surfaceCaps.currentTransform;
  }

  // Find a supported composite alpha format (not all devices support alpha opaque)
  VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  // Simply select the first composite alpha format available
  std::vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
  };
  for (auto& compositeAlphaFlag : compositeAlphaFlags) {
    if (surfaceCaps.supportedCompositeAlpha & compositeAlphaFlag) {
      compositeAlpha = compositeAlphaFlag;
      break;
    };
  }

  VkSwapchainCreateInfoKHR swapchainCI = {};
  swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchainCI.surface = _surface;
  swapchainCI.minImageCount = desireImages;
  swapchainCI.imageFormat = _colorFormat;
  swapchainCI.imageColorSpace = _colorSpace;
  swapchainCI.imageExtent = {chainExtent.width, chainExtent.height};
  swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
  swapchainCI.imageArrayLayers = 1;
  swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchainCI.queueFamilyIndexCount = 0;
  swapchainCI.presentMode = chainPresentMode;
  // Setting oldSwapChain to the saved handle of the previous swapchain aids in resource reuse 
  // and makes sure that we can still present already acquired images
  swapchainCI.oldSwapchain = oldchain;
  // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
  swapchainCI.clipped = VK_TRUE;
  swapchainCI.compositeAlpha = compositeAlpha;

  // Enable transfer source on swap chain images if supported
  if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) {
    swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  // Enable transfer destination on swap chain images if supported
  if (surfaceCaps.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT) {
    swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  vkCreateSwapchainKHR(*_device, &swapchainCI, nullptr, &_swapChain);

  // If an existing swap chain is re-created, destroy the old swap chain
  // This also cleans up all the presentable images
  if (oldchain != VK_NULL_HANDLE) {
    for (uint32_t i = 0; i < _buffers.size(); i++) {
      vkDestroyImageView(*_device, _buffers[i].view, nullptr);
    }
    vkDestroySwapchainKHR(*_device, oldchain, nullptr);
  }

  uint32_t imageCount;
  VK_CHECK_RESULT(vkGetSwapchainImagesKHR(*_device, _swapChain, &imageCount, NULL));
  _images.resize(imageCount);
  VK_CHECK_RESULT(vkGetSwapchainImagesKHR(*_device, _swapChain, &imageCount, _images.data()));

  // Get the swap chain buffers containing the image and imageview
  _buffers.resize(imageCount);
  for (uint32_t i = 0; i < imageCount; i++) {
    VkImageViewCreateInfo colorAttachmentView = {};
    colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    colorAttachmentView.pNext = NULL;
    colorAttachmentView.format = _colorFormat;
    colorAttachmentView.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A};
    colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    colorAttachmentView.subresourceRange.baseMipLevel = 0;
    colorAttachmentView.subresourceRange.levelCount = 1;
    colorAttachmentView.subresourceRange.baseArrayLayer = 0;
    colorAttachmentView.subresourceRange.layerCount = 1;
    colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    colorAttachmentView.flags = 0;

    _buffers[i].image = _images[i];

    colorAttachmentView.image = _buffers[i].image;

    VK_CHECK_RESULT(vkCreateImageView(*_device, &colorAttachmentView, nullptr, &_buffers[i].view));
  }
}

VulkanSwapChain::DepthImage VulkanSwapChain::create_depth_image(uint32_t width, uint32_t height)
{
  DepthImage unit = {};

  // Create an optimal image used as the depth stencil attachment
  VkImageCreateInfo image = {};
  image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image.imageType = VK_IMAGE_TYPE_2D;
  image.format = _depthFormat;
  image.extent = {width, height, 1};
  image.mipLevels = 1;
  image.arrayLayers = 1;
  image.samples = VK_SAMPLE_COUNT_1_BIT;
  image.tiling = VK_IMAGE_TILING_OPTIMAL;
  image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  VK_CHECK_RESULT(vkCreateImage(*_device, &image, nullptr, &unit.image));

  // Allocate memory for the image (device local) and bind it to our image
  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(*_device, unit.image, &memReqs);
  memAlloc.allocationSize = memReqs.size;
  auto memIndex = _device->memory_type_index(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  if (!memIndex)
    throw std::runtime_error("No proper memory type!");
  memAlloc.memoryTypeIndex = *memIndex; 
  VK_CHECK_RESULT(vkAllocateMemory(*_device, &memAlloc, nullptr, &unit.mem));
  VK_CHECK_RESULT(vkBindImageMemory(*_device, unit.image, unit.mem, 0));

  // Create a view for the depth stencil image
  // Images aren't directly accessed in Vulkan, but rather through views described by a subresource range
  // This allows for multiple views of one image with differing ranges (e.g. for different layers)
  VkImageViewCreateInfo depthStencilView = {};
  depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
  depthStencilView.format = _depthFormat;
  depthStencilView.subresourceRange = {};
  depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  // Stencil aspect should only be set on depth + stencil formats (VK_FORMAT_D16_UNORM_S8_UINT..VK_FORMAT_D32_SFLOAT_S8_UINT)
  if (_depthFormat >= VK_FORMAT_D16_UNORM_S8_UINT)
    depthStencilView.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

  depthStencilView.subresourceRange.baseMipLevel = 0;
  depthStencilView.subresourceRange.levelCount = 1;
  depthStencilView.subresourceRange.baseArrayLayer = 0;
  depthStencilView.subresourceRange.layerCount = 1;
  depthStencilView.image = unit.image;
  VK_CHECK_RESULT(vkCreateImageView(*_device, &depthStencilView, nullptr, &unit.view));

  return unit;
}

std::vector<VkFramebuffer> VulkanSwapChain::create_frame_buffer(VkRenderPass vkPass, const DepthImage& depthUnit) 
{
  assert(_buffers.size() == _images.size());

  VkImageView attachments[2];

  // Depth/Stencil attachment is the same for all frame buffers
  attachments[1] = depthUnit.view;

  VkFramebufferCreateInfo frameBufferCreateInfo = {};
  frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  frameBufferCreateInfo.pNext = NULL;
  frameBufferCreateInfo.renderPass = vkPass;
  frameBufferCreateInfo.attachmentCount = 2;
  frameBufferCreateInfo.pAttachments = attachments;
  frameBufferCreateInfo.width = _width;
  frameBufferCreateInfo.height = _height;
  frameBufferCreateInfo.layers = 1;

  std::vector<VkFramebuffer> frameBuffers;
  // Create frame buffers for every swap chain image
  frameBuffers.resize(_buffers.size());
  for (uint32_t i = 0; i < frameBuffers.size(); i++) {
    attachments[0] = _buffers[i].view;
    VK_CHECK_RESULT(vkCreateFramebuffer(*_device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
  }

  return frameBuffers;
}

std::tuple<VkResult, uint32_t> VulkanSwapChain::acquire_image(VkSemaphore present_sema)
{
  uint32_t index;
  auto result = vkAcquireNextImageKHR(*_device, _swapChain, UINT64_MAX, present_sema, nullptr, &index);
  return {result, index};
}

VkResult VulkanSwapChain::queue_present(VkQueue queue, uint32_t index, VkSemaphore wait_sema)
{
  VkPresentInfoKHR presentInfo = {};
  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = NULL;
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &_swapChain;
  presentInfo.pImageIndices = &index;
  // Check if a wait semaphore has been specified to wait for before presenting the image
  if (wait_sema != VK_NULL_HANDLE) {
    presentInfo.pWaitSemaphores = &wait_sema;
    presentInfo.waitSemaphoreCount = 1;
  }
  return vkQueuePresentKHR(queue, &presentInfo);
}
