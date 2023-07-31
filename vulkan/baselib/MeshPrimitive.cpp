#include "MeshPrimitive.h"
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "VulkanBuffer.h"

#include "config.h"
#include "RenderData.h"

#define SHADER_DIR ROOT_DIR##"vulkan/baselib"

using tg::vec2;
using tg::vec3;

MeshPrimitive::MeshPrimitive()
{
}

MeshPrimitive::~MeshPrimitive()
{
}

void MeshPrimitive::set_transform(const tg::mat4& m)
{
  _m = m;
}

void MeshPrimitive::set_vertex(uint8_t* data, int n)
{
  int sz = n / sizeof(tg::vec3);
  _vertexs.resize(sz);
  memcpy(_vertexs.data(), data, n);
}

void MeshPrimitive::set_normal(uint8_t* data, int n)
{
  int sz = n / sizeof(tg::vec3);
  _normals.resize(sz);
  memcpy(_normals.data(), data, n);
}

void MeshPrimitive::set_uvs(uint8_t* data, int n)
{
  int sz = n / sizeof(tg::vec2);
  _uvs.resize(sz);
  memcpy(_uvs.data(), data, n);
}

void MeshPrimitive::set_index(uint8_t* data, int n)
{
  int sz = n / 2;
  _indexs.resize(sz);
  memcpy(_indexs.data(), data, n);
}

uint32_t MeshPrimitive::index_count()
{
  return _indexs.size();
}

void MeshPrimitive::set_material(const Material& m)
{
  _material = m;
}

void MeshPrimitive::realize(const std::shared_ptr<VulkanDevice>& dev)
{
  auto fun = [dev](uint8_t* data, int n) -> std::shared_ptr<VulkanBuffer> {
    auto ori_buf = dev->create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, n, data);
    auto dst_buf = dev->create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, n, 0);
    dev->copy_buffer(ori_buf.get(), dst_buf.get(), dev->transfer_queue());
    return dst_buf;
  };
  _vertex_buf = fun((uint8_t *)_vertexs.data(), _vertexs.size() * sizeof(vec3));
  _normal_buf = fun((uint8_t *)_normals.data(), _normals.size() * sizeof(vec3));
  _uv_buf = fun((uint8_t *)_uvs.data(), _uvs.size() * sizeof(vec3));

  auto index_sz = _indexs.size() * sizeof(uint16_t);
  auto index_ori_buf = dev->create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, index_sz, (uint8_t *)_indexs.data());
  auto index_buf = dev->create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, index_sz, 0);
  dev->copy_buffer(index_ori_buf.get(), index_buf.get(), dev->transfer_queue());
  _index_buf = index_buf;
}

//void MeshPrimitive::build_command_buffer(VkCommandBuffer cmd_buf, VkPipelineLayout pipelayout)
//{
//  if (!_pipeline)
//    return;
//
//  vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
//
//  //vkCmdPushConstants(cmd_buf, pipelayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PrimitiveData), &_m);
//
//  if(_material.tex)
//    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelayout, 1, 1, &_material_set, 0, nullptr);
//
//  std::vector<VkBuffer> bufs(_attr_bufs.size());
//  for (int i = 0; i < _attr_bufs.size(); i++) {
//    bufs[_input_attr[i].location] = *_attr_bufs[i];
//  }
//  std::vector<VkDeviceSize> offset(_attr_bufs.size(), 0);
//  vkCmdBindVertexBuffers(cmd_buf, 0, bufs.size(), bufs.data(), offset.data());
//  vkCmdBindIndexBuffer(cmd_buf, *_index_buf, 0, _index_type);
//  vkCmdDrawIndexed(cmd_buf, _index_count, 1, 0, 0, 0);
//}

