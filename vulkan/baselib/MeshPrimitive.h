#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>

#include "tvec.h"

class VulkanBuffer;
class VulkanDevice;

class MeshPrimitive {
  friend class GLTFLoader;
  friend class MeshInstance;

public:
  MeshPrimitive(std::shared_ptr<VulkanDevice> &dev);
  ~MeshPrimitive();

  void set_matrix(const tg::mat4 &m);

  void build_command_buffer(VkCommandBuffer cmd_buf, VkPipelineLayout pipelayout);

private:

  void add_vertex_buf(uint8_t *data, int n);

  void set_index_buf(uint8_t *data, int n);

  void create_pipeline(VkRenderPass renderpass, VkPipelineLayout pipelayout, std::vector<VkPipelineShaderStageCreateInfo> &shaders);

private:
  std::shared_ptr<VulkanDevice> _device;

  VkPrimitiveTopology _mode;
  VkPipeline _pipeline = VK_NULL_HANDLE;

  tg::mat4 _m;

  VkIndexType _index_type;
  uint32_t _index_count;
  std::shared_ptr<VulkanBuffer> _index_buf;

  std::vector<std::shared_ptr<VulkanBuffer>> _attr_bufs;
  std::vector<VkVertexInputBindingDescription> _input_bind;
  std::vector<VkVertexInputAttributeDescription> _input_attr;
};