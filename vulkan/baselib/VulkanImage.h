#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>

class VulkanDevice;

class VulkanImage {
public:
  VulkanImage(std::shared_ptr<VulkanDevice> &dev);
  ~VulkanImage();

  VkImage& image() { return _image; }
  VkImageView& image_view() { return _image_view; }
  VkDeviceMemory& image_mem() { return _image_mem; }

private:
  std::shared_ptr<VulkanDevice> _device;

  VkImage _image = VK_NULL_HANDLE;
  VkImageView _image_view = VK_NULL_HANDLE;
  VkDeviceMemory _image_mem = VK_NULL_HANDLE;
};
