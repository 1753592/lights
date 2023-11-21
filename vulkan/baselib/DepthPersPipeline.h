#pragma once

#include "VulkanPipeline.h"

class DepthPersPipeline : public VulkanPipeline{
  typedef VulkanPipeline Base;
public:
  DepthPersPipeline(const std::shared_ptr<VulkanDevice> &dev, int w = 1024, int h = 1024);
  ~DepthPersPipeline();

  void realize(VulkanPass *render_pass, int subpass = 0);

  VkPipelineLayout pipe_layout();

private:

  VkPipelineLayout  create_pipe_layout();

private:

  int _w = 0, _h = 0;
}; 