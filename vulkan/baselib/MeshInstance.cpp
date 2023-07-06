#include "MeshInstance.h"

#include "VulkanTools.h"
#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "MeshPrimitive.h"
#include "VulkanInitializers.hpp"

#include "tvec.h"
#include "config.h"

#define SHADER_DIR ROOT_DIR##"/vulkan/baselib/shaders"


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
  writeDescriptorSet.dstSet = _common_set;
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

  if (_common_layout)
    vkDestroyDescriptorSetLayout(*_device, _common_layout, nullptr);

  if (_frag_layout)
    vkDestroyDescriptorSetLayout(*_device, _frag_layout, nullptr);

  if (_descriptor_pool)
    vkDestroyDescriptorPool(*_device, _descriptor_pool, nullptr);
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

void MeshInstance::create_pipeline(VkRenderPass renderpass)
{
  VkPipelineShaderStageCreateInfo shaderStages[4] = {};
  {
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = _device->create_shader(SHADER_DIR "/pbr_clr.vert.spv");
    shaderStages[0].pName = "main";
  }
  {
    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = _device->create_shader(SHADER_DIR "/pbr_clr.frag.spv");
    shaderStages[1].pName = "main";
  }
  {
    shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[2].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[2].module = _device->create_shader(SHADER_DIR "/pbr_tex.vert.spv");
    shaderStages[2].pName = "main";
  }
  {
    shaderStages[3].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[3].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[3].module = _device->create_shader(SHADER_DIR "/pbr_tex.frag.spv");
    shaderStages[3].pName = "main";
  }

  std::vector<VkPipelineShaderStageCreateInfo> shaders(2);
  for (auto pri : _pris) {
    if (pri->material().tex) {
      shaders[0] = shaderStages[2];
      shaders[1] = shaderStages[3];
    }
    else {
      shaders[0] = shaderStages[0];
      shaders[1] = shaderStages[1];
    }
    pri->create_pipeline(renderpass, _pipe_layout, shaders);
  }

  vkDestroyShaderModule(*_device, shaderStages[0].module, nullptr);
  vkDestroyShaderModule(*_device, shaderStages[1].module, nullptr);
  vkDestroyShaderModule(*_device, shaderStages[2].module, nullptr);
  vkDestroyShaderModule(*_device, shaderStages[3].module, nullptr);
}

void MeshInstance::create_pipeline(VkRenderPass renderpass, std::vector<VkPipelineShaderStageCreateInfo> &shaders)
{
  for(auto pri : _pris) {
    pri->create_pipeline(renderpass, _pipe_layout, shaders);
  }
}

void MeshInstance::add_primitive(std::shared_ptr<MeshPrimitive>& pri) {
  _pris.emplace_back(pri);

  auto &m = pri->material();
  if (m.tex) {
 
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = _descriptor_pool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &_frag_layout;

    VkDescriptorSet material_set; 
    VK_CHECK_RESULT(vkAllocateDescriptorSets(*_device, &allocInfo, &material_set));

    VkDescriptorImageInfo descriptor;
    descriptor.imageView = m.tex->image_view();
    descriptor.sampler = m.tex->sampler();
    descriptor.imageLayout = m.tex->image_layout();

    VkWriteDescriptorSet set = {};
    set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    set.dstSet = material_set; 
    set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    set.dstBinding = 0;
    set.pImageInfo = &descriptor;
    set.descriptorCount = 1;

    vkUpdateDescriptorSets(*_device, 1, &set, 0, nullptr);

    pri->set_material_set(material_set);
  }
}

void MeshInstance::build_command_buffer(VkCommandBuffer cmd_buf)
{
  VkDescriptorSet set[2] = {_common_set, 0};
  vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipe_layout, 0, 1, set, 0, nullptr);

  for (auto &pri : _pris) {
    //vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipe_layout, 0, 1, set, 0, nullptr);
    pri->build_command_buffer(cmd_buf, _pipe_layout);
  }
}

void MeshInstance::create_pipe_layout()
{
  {
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*_device, &descriptorLayout, nullptr, &_common_layout));
  }

  {
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = 1;
    descriptorLayout.pBindings = &layoutBinding;

    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*_device, &descriptorLayout, nullptr, &_frag_layout));

  }
  
  VkDescriptorSetLayout deslayout[] = {_common_layout, _frag_layout};
  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
  pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pPipelineLayoutCreateInfo.pNext = nullptr;
  pPipelineLayoutCreateInfo.setLayoutCount = 2;
  pPipelineLayoutCreateInfo.pSetLayouts = deslayout;

  auto pushConstant = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PrimitiveData), 0);
  pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pPipelineLayoutCreateInfo.pPushConstantRanges = &pushConstant;

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
  allocInfo.pSetLayouts = &_common_layout;

  VK_CHECK_RESULT(vkAllocateDescriptorSets(*_device, &allocInfo, &_common_set));
}


