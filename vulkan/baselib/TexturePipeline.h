#pragma once

#include "BasicPipeline.h"

class TexturePipeline : public BasicPipeline{
public:
  TexturePipeline(const std::shared_ptr<VulkanDevice> &dev);
  ~TexturePipeline();

  void realize(VkRenderPass render_pass, int subpass = 0);
};