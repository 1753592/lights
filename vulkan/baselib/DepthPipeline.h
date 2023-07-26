#pragma once

#include "BasicPipeline.h"

class DepthPipeline : public BasicPipeline{
public:
  DepthPipeline(const std::shared_ptr<VulkanDevice> &dev, int w = 1024, int h = 1024);
  ~DepthPipeline();

  void realize(VkRenderPass render_pass, int subpass = 0);

private:
  void initialize();

  int _w = 0, _h = 0;
}; 