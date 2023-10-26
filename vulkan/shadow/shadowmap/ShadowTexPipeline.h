#pragma once

#include "ShadowPipeline.h"

class ShadowTexPipeline : public ShadowPipeline {
public:
  ShadowTexPipeline(const std::shared_ptr<VulkanDevice> &dev);
  ~ShadowTexPipeline();

  void realize(VulkanPass *render_pass, int subpass = 0);

  virtual VkPipelineLayout pipe_layout();

protected:

  VkDescriptorSetLayout shadow_layout();

};