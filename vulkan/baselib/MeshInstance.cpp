#include "MeshInstance.h"

#include "VulkanTools.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "MeshPrimitive.h"
#include "VulkanInitializers.hpp"
#include "TexturePipeline.h"

#include "tvec.h"
#include "config.h"

#define SHADER_DIR ROOT_DIR##"/vulkan/baselib/shaders"


MeshInstance::MeshInstance()
{
  _transform = tg::mat4::identity();
}

MeshInstance::~MeshInstance()
{
}

void MeshInstance::set_transform(const tg::mat4 &transform)
{
  _transform = transform;
}

void MeshInstance::add_primitive(std::shared_ptr<MeshPrimitive>& pri) {
  _pris.emplace_back(pri);

  auto &m = pri->material();
  //if (m.tex) {

  //  VkDescriptorSet material_set; 
  //  VK_CHECK_RESULT(vkAllocateDescriptorSets(*_device, &allocInfo, &material_set));

  //  VkDescriptorImageInfo descriptor;
  //  descriptor.imageView = m.tex->image_view();
  //  descriptor.sampler = m.tex->sampler();
  //  descriptor.imageLayout = m.tex->image_layout();

  //  VkWriteDescriptorSet set = {};
  //  set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //  set.dstSet = material_set; 
  //  set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //  set.dstBinding = 0;
  //  set.pImageInfo = &descriptor;
  //  set.descriptorCount = 1;

  //  vkUpdateDescriptorSets(*_device, 1, &set, 0, nullptr);

  //  pri->set_material_set(material_set);
  //}
}

void MeshInstance::realize(const std::shared_ptr<VulkanDevice> &dev)
{
  //if (!_descriptor_pool) {
  //  VkDescriptorPoolSize pooltype;
  //  pooltype.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //  pooltype.descriptorCount = _pris.size() * 10;

  //  VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
  //  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  //  descriptorPoolInfo.pNext = nullptr;
  //  descriptorPoolInfo.poolSizeCount = 1;
  //  descriptorPoolInfo.pPoolSizes = &pooltype;
  //  descriptorPoolInfo.maxSets = _pris.size() * 10;

  //  VkDescriptorPool desPool;
  //  VK_CHECK_RESULT(vkCreateDescriptorPool(*_device, &descriptorPoolInfo, nullptr, &desPool));
  //  _descriptor_pool = desPool;
  //}

  //VkDescriptorSetAllocateInfo allocInfo = {};
  //allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  //allocInfo.descriptorPool = desPool;
  //allocInfo.descriptorSetCount = 1;
  //allocInfo.pSetLayouts = &mlayout;

  //VK_CHECK_RESULT(vkAllocateDescriptorSets(*device(), &allocInfo, &_matrix_set));

  //VkDescriptorBufferInfo descriptor = {};
  //descriptor.buffer = *_mvp_buf;
  //descriptor.offset = 0;
  //descriptor.range = sz;

  //VkWriteDescriptorSet writeDescriptorSet = {};
  //writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //writeDescriptorSet.dstSet = _matrix_set;
  //writeDescriptorSet.descriptorCount = 1;
  //writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  //writeDescriptorSet.pBufferInfo = &descriptor;
  //writeDescriptorSet.dstBinding = 0;
  //vkUpdateDescriptorSets(*device(), 1, &writeDescriptorSet, 0, nullptr);

  _device = dev;

  for(auto &pri : _pris) {
    pri->realize(dev);
  }
}

void MeshInstance::build_command_buffer(VkCommandBuffer cmd_buf, const std::shared_ptr<TexturePipeline> &pipeline)
{
  if (!pipeline || !pipeline->valid())
    return;

  vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

  for (auto &pri : _pris) {
    auto m = _transform * pri->transform();
    vkCmdPushConstants(cmd_buf, pipeline->pipe_layout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(m), &m);

    std::vector<VkBuffer> bufs(3);
    bufs[0] = *pri->_vertex_buf;
    bufs[1] = *pri->_normal_buf;
    bufs[2] = *pri->_uv_buf;
    std::vector<VkDeviceSize> offset(bufs.size(), 0);
    vkCmdBindVertexBuffers(cmd_buf, 0, bufs.size(), bufs.data(), offset.data());
    vkCmdBindIndexBuffer(cmd_buf, *pri->_index_buf, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd_buf, pri->index_count(), 1, 0, 0, 0);
  }
}

