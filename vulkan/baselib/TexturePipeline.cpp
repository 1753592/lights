#include "TexturePipeline.h"

#include "VulkanTools.h"

#include "tvec.h"
#include "config.h"

using tg::vec2;
using tg::vec3;
using tg::vec4;

#define SHADER_DIR ROOT_DIR##"/vulkan/baselib/shaders"

TexturePipeline::TexturePipeline(const std::shared_ptr<VulkanDevice> &dev) : BasicPipeline(dev) 
{
}

TexturePipeline::~TexturePipeline()
{

}

void TexturePipeline::realize(VkRenderPass render_pass, int subpass)
{
  initialize();

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
  pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipelineCreateInfo.layout = _pipe_layout;
  pipelineCreateInfo.renderPass = render_pass;

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
  inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineRasterizationStateCreateInfo rasterizationState = {};
  rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
  rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
  rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizationState.depthClampEnable = VK_FALSE;
  rasterizationState.rasterizerDiscardEnable = VK_FALSE;
  rasterizationState.depthBiasEnable = VK_FALSE;
  rasterizationState.lineWidth = 1.0f;

  VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
  blendAttachmentState[0].colorWriteMask = 0xf;
  blendAttachmentState[0].blendEnable = VK_FALSE;
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

  VkVertexInputBindingDescription vertexInputBindings[3] = {};
  vertexInputBindings[0].binding = 0;  // vkCmdBindVertexBuffers
  vertexInputBindings[0].stride = sizeof(vec3);
  vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertexInputBindings[1].binding = 1;  // vkCmdBindVertexBuffers
  vertexInputBindings[1].stride = sizeof(vec3);
  vertexInputBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  vertexInputBindings[2].binding = 2;
  vertexInputBindings[2].stride = sizeof(vec2);
  vertexInputBindings[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

  VkVertexInputAttributeDescription vertexInputAttributs[3] = {};
  vertexInputAttributs[0].binding = 0;
  vertexInputAttributs[0].location = 0;
  vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttributs[0].offset = 0;
  vertexInputAttributs[1].binding = 1;
  vertexInputAttributs[1].location = 1;
  vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  vertexInputAttributs[1].offset = 0;
  vertexInputAttributs[2].binding = 2;
  vertexInputAttributs[2].location = 2;
  vertexInputAttributs[2].format = VK_FORMAT_R32G32_SFLOAT; 
  vertexInputAttributs[2].offset = 0;

  VkPipelineVertexInputStateCreateInfo vertexInputState = {};
  vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputState.vertexBindingDescriptionCount = 3;
  vertexInputState.pVertexBindingDescriptions = vertexInputBindings;
  vertexInputState.vertexAttributeDescriptionCount = 3;
  vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs;

  VkPipelineShaderStageCreateInfo shaderStages[2] = {};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = _device->create_shader(SHADER_DIR "/pbr_tex.vert.spv");
  shaderStages[0].pName = "main";
  assert(shaderStages[0].module != VK_NULL_HANDLE);

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = _device->create_shader(SHADER_DIR "/pbr_tex.frag.spv");
  shaderStages[1].pName = "main";
  assert(shaderStages[1].module != VK_NULL_HANDLE);

  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStages;

  pipelineCreateInfo.pVertexInputState = &vertexInputState;
  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState = &colorBlendState;
  pipelineCreateInfo.pMultisampleState = &multisampleState;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pDepthStencilState = &depthStencilState;
  pipelineCreateInfo.pDynamicState = &dynamicState;
  pipelineCreateInfo.subpass = subpass;

  VK_CHECK_RESULT(vkCreateGraphicsPipelines(*_device, _device->get_or_create_pipecache(), 1, &pipelineCreateInfo, nullptr, &_pipeline));

  vkDestroyShaderModule(*_device, shaderStages[0].module, nullptr);
  vkDestroyShaderModule(*_device, shaderStages[1].module, nullptr);
}

