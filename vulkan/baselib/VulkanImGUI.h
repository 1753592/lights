#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class VulkanDevice;

class VulkanImGUI{
public:
  VulkanImGUI(std::shared_ptr<VulkanDevice> dev, VkSurfaceKHR surface);
  ~VulkanImGUI();

private:
  std::shared_ptr<VulkanDevice> _device;
};