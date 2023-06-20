#include "VulkanImGUI.h"

#include "VulkanDevice.h"
#include "VulkanBuffer.h"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"

#include "imgui/imgui.h"

#include "tvec.h"
#include "shaders/hud.vert.spv"
#include "shaders/hud.frag.spv"

namespace {
struct PushConstBlock {
  tg::vec2 scale;
  tg::vec2 translate;
} const_block;
}  // namespace

VulkanImGUI::VulkanImGUI(std::shared_ptr<VulkanDevice> dev) : _device(dev)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  ImGui::StyleColorsLight();

  io.Fonts->AddFontFromFileTTF("c:/Windows/fonts/msyh.ttc", 16);
  unsigned char* data = 0;
  int texWidth, texHeight;
  io.Fonts->GetTexDataAsRGBA32(&data, &texWidth, &texHeight);

  std::tie(_font_tex, _font_memory) = _device->create_image(texWidth, texHeight);
  _font_view = _device->create_image_view(_font_tex);
}

VulkanImGUI::~VulkanImGUI()
{
  if (_pipeline)
    vkDestroyPipeline(*_device, _pipeline, nullptr);
  if (_descriptor_pool)
    vkDestroyDescriptorPool(*_device, _descriptor_pool, nullptr);
  if (_sampler)
    vkDestroySampler(*_device, _sampler, nullptr);
}

void VulkanImGUI::resize(int w, int h)
{
  auto &io = ImGui::GetIO();
  io.DisplaySize = ImVec2(w, h);
}

void VulkanImGUI::create_pipeline(VkRenderPass render_pass, VkFormat clrformat, VkFormat depformat)
{
  VkSamplerCreateInfo samplerInfo = vks::initializers::samplerCreateInfo();
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
  samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
  VK_CHECK_RESULT(vkCreateSampler(*_device, &samplerInfo, nullptr, &_sampler));

  // Descriptor pool
  std::vector<VkDescriptorPoolSize> poolSizes = {vks::initializers::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)};
  VkDescriptorPoolCreateInfo descriptorPoolInfo = vks::initializers::descriptorPoolCreateInfo(poolSizes, 2);
  VK_CHECK_RESULT(vkCreateDescriptorPool(*_device, &descriptorPoolInfo, nullptr, &_descriptor_pool));

  // Descriptor set layout
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
      vks::initializers::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
  };

  VkDescriptorSetLayout descriptor_layout;
  VkDescriptorSetLayoutCreateInfo descriptorLayout = vks::initializers::descriptorSetLayoutCreateInfo(setLayoutBindings);
  VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*_device, &descriptorLayout, nullptr, &descriptor_layout));

  VkDescriptorSetAllocateInfo allocInfo = vks::initializers::descriptorSetAllocateInfo(_descriptor_pool, &descriptor_layout, 1);
  VK_CHECK_RESULT(vkAllocateDescriptorSets(*_device, &allocInfo, &_descriptor_set));
  VkDescriptorImageInfo fontDescriptor = vks::initializers::descriptorImageInfo(_sampler, _font_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  std::vector<VkWriteDescriptorSet> writeDescriptorSets = {
      vks::initializers::writeDescriptorSet(_descriptor_set, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &fontDescriptor)};
  vkUpdateDescriptorSets(*_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

  VkPipelineLayout pipeLayout;
  VkPushConstantRange pushConstantRange = vks::initializers::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock), 0);
  VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = vks::initializers::pipelineLayoutCreateInfo(&descriptor_layout, 1);
  pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
  pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
  VK_CHECK_RESULT(vkCreatePipelineLayout(*_device, &pipelineLayoutCreateInfo, nullptr, &pipeLayout));
  _pipe_layout = pipeLayout;

  //-----------------------------------------------------------------------------------------------------------

  // Push constants for UI rendering parameters
  VkPipelineCache pipe_cache = _device->get_or_create_pipecache();

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
      vks::initializers::pipelineInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);

  VkPipelineRasterizationStateCreateInfo rasterizationState =
      vks::initializers::pipelineRasterizationStateCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

  VkPipelineColorBlendAttachmentState blendAttachmentState{};
  blendAttachmentState.blendEnable = VK_TRUE;
  blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
  blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
  blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
  blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
  blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

  VkPipelineColorBlendStateCreateInfo colorBlendState = vks::initializers::pipelineColorBlendStateCreateInfo(1, &blendAttachmentState);

  VkPipelineDepthStencilStateCreateInfo depthStencilState = vks::initializers::pipelineDepthStencilStateCreateInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

  VkPipelineViewportStateCreateInfo viewportState = vks::initializers::pipelineViewportStateCreateInfo(1, 1, 0);

  VkPipelineMultisampleStateCreateInfo multisampleState = vks::initializers::pipelineMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);

  std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
  VkPipelineDynamicStateCreateInfo dynamicState = vks::initializers::pipelineDynamicStateCreateInfo(dynamicStateEnables);

  VkGraphicsPipelineCreateInfo pipelineCreateInfo = vks::initializers::pipelineCreateInfo(pipeLayout, render_pass);

  VkPipelineShaderStageCreateInfo shaderStages[2] = {};
  shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
  shaderStages[0].module = _device->create_shader(hud_vert, sizeof(hud_vert));
  shaderStages[0].pName = "main";
  assert(shaderStages[0].module != VK_NULL_HANDLE);

  shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
  shaderStages[1].module = _device->create_shader(hud_frag, sizeof(hud_frag));
  shaderStages[1].pName = "main";
  assert(shaderStages[1].module != VK_NULL_HANDLE);

  pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
  pipelineCreateInfo.pRasterizationState = &rasterizationState;
  pipelineCreateInfo.pColorBlendState = &colorBlendState;
  pipelineCreateInfo.pMultisampleState = &multisampleState;
  pipelineCreateInfo.pViewportState = &viewportState;
  pipelineCreateInfo.pDepthStencilState = &depthStencilState;
  pipelineCreateInfo.pDynamicState = &dynamicState;
  pipelineCreateInfo.stageCount = 2;
  pipelineCreateInfo.pStages = shaderStages;
  pipelineCreateInfo.subpass = 0;

#if defined(VK_KHR_dynamic_rendering)
  // SRS - if we are using dynamic rendering (i.e. renderPass null), must define color, depth and stencil attachments at pipeline create time
  VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo = {};
  if (render_pass == VK_NULL_HANDLE) {
    pipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    pipelineRenderingCreateInfo.colorAttachmentCount = 1;
    pipelineRenderingCreateInfo.pColorAttachmentFormats = &clrformat;
    pipelineRenderingCreateInfo.depthAttachmentFormat = depformat;
    pipelineRenderingCreateInfo.stencilAttachmentFormat = depformat;
    pipelineCreateInfo.pNext = &pipelineRenderingCreateInfo;
  }
#endif

  // Vertex bindings an attributes based on ImGui vertex definition
  std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
      vks::initializers::vertexInputBindingDescription(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX),
  };
  std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
      vks::initializers::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),
      vks::initializers::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),
      vks::initializers::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),
  };
  VkPipelineVertexInputStateCreateInfo vertexInputState = vks::initializers::pipelineVertexInputStateCreateInfo();
  vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
  vertexInputState.pVertexBindingDescriptions = vertexInputBindings.data();
  vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
  vertexInputState.pVertexAttributeDescriptions = vertexInputAttributes.data();

  pipelineCreateInfo.pVertexInputState = &vertexInputState;

  VK_CHECK_RESULT(vkCreateGraphicsPipelines(*_device, pipe_cache, 1, &pipelineCreateInfo, nullptr, &_pipeline));


  vkDestroyShaderModule(*_device, shaderStages[0].module, nullptr);
  vkDestroyShaderModule(*_device, shaderStages[1].module, nullptr);
  vkDestroyDescriptorSetLayout(*_device, descriptor_layout, 0);
}

bool VulkanImGUI::update()
{
  ImDrawData* imDrawData = ImGui::GetDrawData();
  bool updateCmdBuffers = false;

  if (!imDrawData) { return false; };

  VkDeviceSize vertex_buf_size = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize index_buf_size = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

  if (vertex_buf_size == 0 || index_buf_size == 0) { return false; }

  if (_vert_buf == 0 || _vert_buf->size() < vertex_buf_size) {
    _vert_buf = _device->create_buffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertex_buf_size, 0);
    updateCmdBuffers = true;
  }

  if (_index_buf == 0 || _index_buf->size() < index_buf_size) {
    _index_buf = _device->create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, index_buf_size, 0);
    updateCmdBuffers = true;
  }

  uint8_t* vtxDst = _vert_buf->map();
  uint8_t* idxDst = _index_buf->map();
  for (int n = 0; n < imDrawData->CmdListsCount; n++) {
    const ImDrawList* cmd_list = imDrawData->CmdLists[n];
    memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += cmd_list->VtxBuffer.Size;
    idxDst += cmd_list->IdxBuffer.Size;
  }
  _vert_buf->flush();
  _index_buf->flush();
  _vert_buf->unmap();
  _index_buf->unmap();

  return updateCmdBuffers;
}

void VulkanImGUI::draw(const VkCommandBuffer cmdbuf)
{
  ImDrawData *imdata = ImGui::GetDrawData();
  if (!imdata || imdata->CmdListsCount == 0)
    return;

  auto io = ImGui::GetIO();

  vkCmdBindPipeline(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
  vkCmdBindDescriptorSets(cmdbuf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipe_layout, 0, 1, &_descriptor_set, 0, nullptr);

  const_block.scale = tg::vec2(2.0 / io.DisplaySize.x, 2.0 / io.DisplaySize.y);
  const_block.translate = tg::vec2(-1.f);

  vkCmdPushConstants(cmdbuf, _pipe_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstBlock), &const_block);

  VkDeviceSize offsets[1] = {};
  vkCmdBindVertexBuffers(cmdbuf, 0, 1, *_vert_buf, offsets);
  vkCmdBindIndexBuffer(cmdbuf, *_index_buf, 0, VK_INDEX_TYPE_UINT16);

  uint32_t indexOffset = 0, vertexOffset = 0;
  for (int32_t i = 0; i < imdata->CmdListsCount; i++) {
    const ImDrawList* cmd_list = imdata->CmdLists[i];
    for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
      VkRect2D scissorRect;
      scissorRect.offset.x = std::max<uint32_t>((int32_t)(pcmd->ClipRect.x), 0);
      scissorRect.offset.y = std::max<uint32_t>((int32_t)(pcmd->ClipRect.y), 0);
      scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
      scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
      vkCmdSetScissor(cmdbuf, 0, 1, &scissorRect);
      vkCmdDrawIndexed(cmdbuf, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
      indexOffset += pcmd->ElemCount;
    }
    vertexOffset += cmd_list->VtxBuffer.Size;
  }
}

void VulkanImGUI::create_canvas()
{
}
