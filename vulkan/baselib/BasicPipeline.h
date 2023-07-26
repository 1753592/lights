#pragma once

#include <vulkan/vulkan_core.h>
#include "VulkanDevice.h"

class BasicPipeline{
public:
  BasicPipeline(const std::shared_ptr<VulkanDevice> &dev);
  ~BasicPipeline();

  operator VkPipeline() { return _pipeline; }

  virtual void realize(VkRenderPass render_pass, int subpass = 0);

  bool valid() { return _pipeline != VK_NULL_HANDLE; }

  VkDescriptorSetLayout descriptor_layout() { return _descriptor_layout; }
  void set_descriptor_layout(VkDescriptorSetLayout layout) { _descriptor_layout = layout; }

  VkPipelineLayout pipe_layout() { return _pipe_layout; }

protected:

  void initialize();
  
  std::shared_ptr<VulkanDevice> _device;

  VkDescriptorSetLayout _descriptor_layout = VK_NULL_HANDLE;
  VkPipelineLayout  _pipe_layout = VK_NULL_HANDLE;
  VkPipeline        _pipeline = VK_NULL_HANDLE;

};