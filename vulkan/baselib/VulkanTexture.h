#pragma once

#include <vulkan/vulkan_core.h>
#include <memory>
#include <string>

class VulkanDevice;

class VulkanTexture {
public:
  VulkanTexture();

  ~VulkanTexture();

  VkImageView image_view() { return _image_view; }

  VkImageLayout image_layout() { return _image_layout; }

  VkSampler sampler() { return _sampler; }

  void load_image(const std::string &file);

  void load_data(int w, int h, int comp, int compsize, uint8_t*data, int n);

private:
  std::shared_ptr<VulkanDevice> _device;

  int _w, _h;

  VkImageLayout _image_layout = VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL;

  VkImage _image = VK_NULL_HANDLE;
  VkImageView _image_view = VK_NULL_HANDLE;
  VkDeviceMemory _image_mem = VK_NULL_HANDLE;

  VkSampler _sampler = VK_NULL_HANDLE;
};