#pragma once

#include <vulkan/vulkan_core.h>
#include "VulkanDevice.h"

class VulkanPass;

class VulkanPipeline{
public:
  VulkanPipeline(const std::shared_ptr<VulkanDevice> &dev);
  ~VulkanPipeline();

  operator VkPipeline() { return _pipeline; }

  virtual void realize(VulkanPass *render_pass, int subpass = 0);

  bool valid() { return _pipeline != VK_NULL_HANDLE; }

  VkDescriptorSetLayout matrix_layout();
  void set_matrix_layout(VkDescriptorSetLayout layout) { _matrix_layout = layout; }

  VkDescriptorSetLayout light_layout();
  void set_light_layout(VkDescriptorSetLayout layout) { _light_layout = layout; }

  VkDescriptorSetLayout pbr_layout();
  void set_pbr_layout(VkDescriptorSetLayout layout) { _pbr_layout = layout; }

  VkPipelineLayout pipe_layout();

protected:
  
  std::shared_ptr<VulkanDevice> _device;

  VkDescriptorSetLayout _matrix_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _light_layout = VK_NULL_HANDLE;
  VkDescriptorSetLayout _pbr_layout = VK_NULL_HANDLE;

  VkPipelineLayout  _pipe_layout = VK_NULL_HANDLE;
  VkPipeline        _pipeline = VK_NULL_HANDLE;

};