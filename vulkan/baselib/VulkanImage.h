#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>

class VulkanDevice;

class VulkanImage {
  friend class VulkanDevice;
public:
  VulkanImage(std::shared_ptr<VulkanDevice> &dev);
  ~VulkanImage();

  const VkImage& image() const { return _image; }
  const VkImageView& image_view() const { return _image_view; }
  const VkDeviceMemory& image_mem() const { return _image_mem; }
  const VkFormat& format() const { return _format; }

private:
  std::shared_ptr<VulkanDevice> _device;

  VkImage _image = VK_NULL_HANDLE;
  VkImageView _image_view = VK_NULL_HANDLE;
  VkDeviceMemory _image_mem = VK_NULL_HANDLE;

  VkFormat _format = VK_FORMAT_UNDEFINED;
};
