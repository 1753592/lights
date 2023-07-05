#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>

#include "tvec.h"
#include "RenderData.h"

class VulkanDevice;
class MeshPrimitive;
class VulkanBuffer;

class MeshInstance{
public:
  MeshInstance(std::shared_ptr<VulkanDevice> &dev);
  ~MeshInstance();

  void set_vp(const tg::mat4 &, const tg::mat4 &, const tg::vec3 &);

  void create_pipeline(VkRenderPass renderpass);

  void create_pipeline(VkRenderPass renderpass, std::vector<VkPipelineShaderStageCreateInfo> &shaders);

  void add_primitive(std::shared_ptr<MeshPrimitive> &pri);

  void build_command_buffer(VkCommandBuffer cmd_buf);

private:

  void create_pipe_layout();

private:

  std::shared_ptr<VulkanDevice> _device;

  VkPipelineLayout _pipe_layout = VK_NULL_HANDLE;

  VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;

  VkDescriptorSetLayout _common_layout = VK_NULL_HANDLE;

  VkDescriptorSetLayout _frag_layout = VK_NULL_HANDLE;

  VkDescriptorSet _common_set = VK_NULL_HANDLE;

  std::shared_ptr<VulkanBuffer> _ubo_buf;

  std::vector<std::shared_ptr<MeshPrimitive>> _pris;

  MVP _mvp;
};