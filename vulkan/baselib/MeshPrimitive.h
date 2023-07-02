#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <memory>

class VulkanBuffer;
class VulkanDevice;

class MeshPrimitive {
  friend class GLTFLoader;
  friend class MeshInstance;

public:
  MeshPrimitive(std::shared_ptr<VulkanDevice> &dev);
  ~MeshPrimitive();

private:

  void add_vertex_buf(uint8_t *data, int n);

  void set_index_buf(uint8_t *data, int n);

private:
  std::shared_ptr<VulkanDevice> _device;

  VkPrimitiveTopology _mode;

  VkIndexType _indices_type;

  std::shared_ptr<VulkanBuffer> _index_buf;
  std::vector<std::shared_ptr<VulkanBuffer>> _attr_bufs;


  std::vector<VkVertexInputBindingDescription> _input_bind;
  std::vector<VkVertexInputAttributeDescription> _input_attr;
};