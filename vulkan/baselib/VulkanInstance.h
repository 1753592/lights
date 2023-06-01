#ifndef __VULKAN_INSTANCE_INC__
#define __VULKAN_INSTANCE_INC__

#include "vulkan/vulkan_core.h"

#include <string>

class VulkanInstance{
public:
  VulkanInstance();
  ~VulkanInstance();

  void enable_debug();

  void create_device(const std::string &dev = "");

private:
  void initialize();

  VkInstance _instance = nullptr;
};

#endif