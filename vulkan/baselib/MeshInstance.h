#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>

class VulkanDevice;
class MeshPrimitive;
class VulkanBuffer;

class MeshInstance{
public:
  MeshInstance(std::shared_ptr<VulkanDevice> &dev);
  ~MeshInstance();

  void add_primitive(std::shared_ptr<MeshPrimitive> &pri);

private:

private:

  std::shared_ptr<VulkanDevice> _device;

  VkPipelineLayout _pipe_layout = VK_NULL_HANDLE;

  std::vector<std::shared_ptr<MeshPrimitive>> _pris;
};