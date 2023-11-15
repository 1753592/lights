#pragma once

#include "VulkanPipeline.h"

class HUDPipeline : public VulkanPipeline {
  typedef VulkanPipeline Base;

public:
  HUDPipeline(const std::shared_ptr<VulkanDevice> &dev);
  ~HUDPipeline();

  void realize(VulkanPass *render_pass);

  VkPipelineLayout pipe_layout();

private:
};