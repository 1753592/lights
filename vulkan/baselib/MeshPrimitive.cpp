#include "MeshPrimitive.h"
#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "VulkanBuffer.h"

#include "config.h"
#include "RenderData.h"

#define SHADER_DIR ROOT_DIR##"vulkan/baselib"

MeshPrimitive::MeshPrimitive(std::shared_ptr<VulkanDevice> &dev) : _device(dev) 
{
}

MeshPrimitive::~MeshPrimitive()
{
  if (_pipeline)
    vkDestroyPipeline(*_device, _pipeline, nullptr);
}

void MeshPrimitive::set_matrix(const tg::mat4& m)
{
  _m = m;
}

void MeshPrimitive::set_material(const Material& m)
{
  _material = m;
}

void MeshPrimitive::set_material_set(VkDescriptorSet set)
{
  _material_set = set;
}

void MeshPrimitive::build_command_buffer(VkCommandBuffer cmd_buf, VkPipelineLayout pipelayout)
{
  if (!_pipeline)
    return;

  vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);

  //vkCmdPushConstants(cmd_buf, pipelayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PrimitiveData), &_m);

  if(_material.tex)
    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelayout, 1, 1, &_material_set, 0, nullptr);

  std::vector<VkBuffer> bufs(_attr_bufs.size());
  for (int i = 0; i < _attr_bufs.size(); i++) {
    bufs[_input_attr[i].location] = *_attr_bufs[i];
  }
  std::vector<VkDeviceSize> offset(_attr_bufs.size(), 0);
  vkCmdBindVertexBuffers(cmd_buf, 0, bufs.size(), bufs.data(), offset.data());
  vkCmdBindIndexBuffer(cmd_buf, *_index_buf, 0, _index_type);
  vkCmdDrawIndexed(cmd_buf, _index_count, 1, 0, 0, 0);
}

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

void MeshPrimitive::create_pipeline(VkRenderPass renderpass, VkPipelineLayout pipelayout, std::vector<VkPipelineShaderStageCreateInfo> &shaders)
{
  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.layout = pipelayout;
  pipelineCreateInfo.renderPass = renderpass;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
  inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyState.topology = _mode;

  VkPipelineRasterizationStateCreateInfo rasterizationState = {};
  rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_NONE;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.rasterizerDiscardEnable = VK_FALSE;
  rasterizationState.depthBiasEnable = VK_FALSE;
  rasterizationState.lineWidth = 1.0f;

  VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
  blendAttachmentState[0].colorWriteMask = 0xf;
  blendAttachmentState[0].blendEnable = VK_TRUE;
  blendAttachmentState[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendAttachmentState[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState[0].colorBlendOp = VK_BLEND_OP_ADD;
  blendAttachmentState[0].alphaBlendOp = VK_BLEND_OP_ADD;
  blendAttachmentState[0].srcAlphaBlendFactor= VK_BLEND_FACTOR_ONE;
  blendAttachmentState[0].dstAlphaBlendFactor= VK_BLEND_FACTOR_ZERO;

  VkPipelineColorBlendStateCreateInfo colorBlendState = {};
  colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  colorBlendState.attachmentCount = 1;
  colorBlendState.pAttachments = blendAttachmentState;

  VkPipelineViewportStateCreateInfo viewportState = {};
  viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount = 1;

  std::vector<VkDynamicState> dynamicStateEnables;
  dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
  dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.pDynamicStates = dynamicStateEnables.data();
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

  VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
  depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  depthStencilState.depthTestEnable = VK_TRUE;
  depthStencilState.depthWriteEnable = VK_TRUE;
  depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  depthStencilState.depthBoundsTestEnable = VK_FALSE;
  depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
  depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
  depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
  depthStencilState.stencilTestEnable = VK_FALSE;
  depthStencilState.front = depthStencilState.back;

  VkPipelineMultisampleStateCreateInfo multisampleState = {};
  multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampleState.pSampleMask = nullptr;

  VkPipelineVertexInputStateCreateInfo vertexInputState = {};
  vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputState.vertexBindingDescriptionCount = _input_bind.size();
  vertexInputState.pVertexBindingDescriptions = _input_bind.data();
  vertexInputState.vertexAttributeDescriptionCount = _input_attr.size();
  vertexInputState.pVertexAttributeDescriptions = _input_attr.data();

  pipelineCreateInfo.stageCount = shaders.size();
  pipelineCreateInfo.pStages = shaders.data();

  pipelineCreateInfo.pVertexInputState = &vertexInputState;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState = &colorBlendState;
  pipelineCreateInfo.pMultisampleState = &multisampleState;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pDepthStencilState = &depthStencilState;
  pipelineCreateInfo.pDynamicState = &dynamicState;

  VK_CHECK_RESULT(vkCreateGraphicsPipelines(*_device, _device->get_or_create_pipecache(), 1, &pipelineCreateInfo, nullptr, &_pipeline));
}
