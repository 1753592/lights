#include "MeshInstance.h"

#include "VulkanTools.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "MeshPrimitive.h"

#include "tvec.h"


MeshInstance::MeshInstance(std::shared_ptr<VulkanDevice> &dev) : _device(dev)
{
  create_pipe_layout();

  _mvp.model = tg::mat4::identity();
  _ubo_buf = _device->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(_mvp));

  VkDescriptorBufferInfo descriptor = {};

  descriptor.buffer = *_ubo_buf;
  descriptor.offset = 0;
  descriptor.range = _ubo_buf->size();

  VkWriteDescriptorSet writeDescriptorSet = {};
  writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeDescriptorSet.dstSet = _matrix_set;
  writeDescriptorSet.descriptorCount = 1;
  writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  writeDescriptorSet.pBufferInfo = &descriptor;
  writeDescriptorSet.dstBinding = 0;

  vkUpdateDescriptorSets(*_device, 1, &writeDescriptorSet, 0, nullptr);
}

MeshInstance::~MeshInstance()
{
  if (_pipe_layout)
    vkDestroyPipelineLayout(*_device, _pipe_layout, nullptr);

  if (_descriptor_layout)
    vkDestroyDescriptorSetLayout(*_device, _descriptor_layout, nullptr);

  if (_descriptor_pool)
    vkDestroyDescriptorPool(*_device, _descriptor_pool, nullptr);
}

void MeshInstance::set_m(const tg::mat4 &m)
{
  _mvp.model = m;
}

void MeshInstance::set_vp(const tg::mat4 &prj, const tg::mat4 &view, const tg::vec3 &eye)
{
  uint8_t *data = 0;
  _mvp.prj = prj;
  _mvp.view = view;
  _mvp.eye = eye;
  VK_CHECK_RESULT(vkMapMemory(*_device, _ubo_buf->memory(), 0, sizeof(_mvp), 0, (void **)&data));
  memcpy(data, &_mvp, sizeof(_mvp));
  vkUnmapMemory(*_device, _ubo_buf->memory());
}

void MeshInstance::create_pipeline(VkRenderPass renderpass, std::vector<VkPipelineShaderStageCreateInfo> &shaders)
{
  for(auto pri : _pris) {
    pri->create_pipeline(renderpass, _pipe_layout, shaders);
  }
}

void MeshInstance::add_primitive(std::shared_ptr<MeshPrimitive>& pri) {
  _pris.emplace_back(pri);
}

void MeshInstance::build_command_buffer(VkCommandBuffer cmd_buf)
{
  VkDescriptorSet set[2] = {_matrix_set, 0};
  vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipe_layout, 0, 1, set, 0, nullptr);

  for (auto &pri : _pris) {

    //vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipe_layout, 0, 1, set, 0, nullptr);

    pri->build_command_buffer(cmd_buf);
  }
}

void MeshInstance::create_pipe_layout()
{
  VkDescriptorSetLayoutBinding layoutBinding[3] = {};
  layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding[0].descriptorCount = 1;
  layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[0].pImmutableSamplers = nullptr;

  layoutBinding[1].binding = 1;
  layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding[1].descriptorCount = 1;
  layoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[1].pImmutableSamplers = nullptr;

  layoutBinding[2].binding = 2;
  layoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layoutBinding[2].descriptorCount = 1;
  layoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
  layoutBinding[2].pImmutableSamplers = nullptr;

  VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
  descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorLayout.pNext = nullptr;
  descriptorLayout.bindingCount = 3;
  descriptorLayout.pBindings = layoutBinding;

  VkDescriptorSetLayout descriptor_layout;
  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*_device, &descriptorLayout, nullptr, &descriptor_layout));
  _descriptor_layout = descriptor_layout;

  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
  pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pPipelineLayoutCreateInfo.pNext = nullptr;
  pPipelineLayoutCreateInfo.setLayoutCount = 1;
  pPipelineLayoutCreateInfo.pSetLayouts = &descriptor_layout;

  VkPipelineLayout pipe_layout;
  VK_CHECK_RESULT(vkCreatePipelineLayout(*_device, &pPipelineLayoutCreateInfo, nullptr, &pipe_layout));
  _pipe_layout = pipe_layout;

  VkDescriptorPoolSize typeCounts[1];
  typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  typeCounts[0].descriptorCount = 10;

  VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.pNext = nullptr;
  descriptorPoolInfo.poolSizeCount = 1;
  descriptorPoolInfo.pPoolSizes = typeCounts;
  descriptorPoolInfo.maxSets = 10;

  VkDescriptorPool desPool;
  VK_CHECK_RESULT(vkCreateDescriptorPool(*_device, &descriptorPoolInfo, nullptr, &desPool));
  _descriptor_pool = desPool;

  //----------------------------------------------------------------------------------------------------

  VkDescriptorSetAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = desPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &descriptor_layout;

  VK_CHECK_RESULT(vkAllocateDescriptorSets(*_device, &allocInfo, &_matrix_set));
}


