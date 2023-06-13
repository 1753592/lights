#pragma once

#include <vulkan/vulkan.h>

class VulkanBuffer {
  friend class VulkanDevice;

public:
  VulkanBuffer();
  ~VulkanBuffer();

  void setupDescriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
  void destroy();

private:
  VkDevice _device;
  VkBuffer _buffer = VK_NULL_HANDLE;
  VkDeviceMemory _memory = VK_NULL_HANDLE;
  VkDescriptorBufferInfo _descriptor;
  VkDeviceSize _size = 0;
  VkDeviceSize _alignment = 0;
  VkBufferUsageFlags _usageFlags;
  VkMemoryPropertyFlags _memoryPropertyFlags;
};