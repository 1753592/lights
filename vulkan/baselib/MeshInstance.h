#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>

#include "tvec.h"
#include "RenderData.h"

class VulkanDevice;
class MeshPrimitive;
class VulkanBuffer;
class TexturePipeline;

class MeshInstance{
public:
  MeshInstance();
  ~MeshInstance();

  void set_transform(const tg::mat4 &);

  void add_primitive(std::shared_ptr<MeshPrimitive> &pri);

  void realize(const std::shared_ptr<VulkanDevice> &dev);

  void build_command_buffer(VkCommandBuffer cmd_buf, const std::shared_ptr<TexturePipeline> &pipeline);

private:

private:

  std::shared_ptr<VulkanDevice> _device;

  tg::mat4 _transform;

  VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;

  std::vector<VkDescriptorSet> _matrix_sets;

  std::vector<std::shared_ptr<MeshPrimitive>> _pris;
};