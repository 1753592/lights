#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

class VulkanDevice;

class MeshInstance{
public:
  MeshInstance(VulkanDevice *dev);
  ~MeshInstance();

  void add_buf(uint8_t *data, int n);

private:

  VulkanDevice *_device = nullptr;

  std::vector<VkBuffer> _bufs;
};