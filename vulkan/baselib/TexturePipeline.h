#pragma once

#include "BasicPipeline.h"

class TexturePipeline : public BasicPipeline{
public:
  TexturePipeline(const std::shared_ptr<VulkanDevice> &dev);
  ~TexturePipeline();

  VkDescriptorSetLayout texture_layout();
  void set_texture_layout(VkDescriptorSetLayout layout) { _texture_layout = layout; }

  void realize(VkRenderPass render_pass, int subpass = 0);

  VkDescriptorSetLayout pbr_layout();

  VkPipelineLayout pipe_layout();

protected:

  VkDescriptorSetLayout _texture_layout = VK_NULL_HANDLE;
};