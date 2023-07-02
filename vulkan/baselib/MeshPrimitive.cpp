#include "MeshPrimitive.h"
#include "VulkanDevice.h"

MeshPrimitive::MeshPrimitive(std::shared_ptr<VulkanDevice> &dev) : _device(dev) {}

MeshPrimitive::~MeshPrimitive() {}


void MeshPrimitive::add_vertex_buf(uint8_t* data, int n)
{
  auto ori_buf = _device->create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, n, data);
  auto dst_buf = _device->create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, n, 0);
  _device->copy_buffer(ori_buf.get(), dst_buf.get(), _device->transfer_queue());
  _attr_bufs.push_back(dst_buf);
}

void MeshPrimitive::set_index_buf(uint8_t* data, int n)
{
  auto ori_buf = _device->create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, n, data);
  auto dst_buf = _device->create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, n, 0);
  _device->copy_buffer(ori_buf.get(), dst_buf.get(), _device->transfer_queue());
  _index_buf = dst_buf;
}

