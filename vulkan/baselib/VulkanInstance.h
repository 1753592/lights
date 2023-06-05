#ifndef __VULKAN_INSTANCE_INC__
#define __VULKAN_INSTANCE_INC__

#include "vulkan/vulkan_core.h"

#include <string>
#include <memory>

class VulkanDevice;

class VulkanInstance{
public:
  VulkanInstance();
  ~VulkanInstance();

  void enable_debug();

  std::shared_ptr<VulkanDevice> create_device(const std::string &dev = "");

private:
  void initialize();

  VkInstance _instance = nullptr;
};

#endif