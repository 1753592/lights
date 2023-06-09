#ifndef __VULKAN_INSTANCE_INC__
#define __VULKAN_INSTANCE_INC__

#include "vulkan/vulkan_core.h"

#include <string>
#include <memory>

class VulkanDevice;
class VulkanSwapChain;

class VulkanInstance : public std::enable_shared_from_this<VulkanInstance>{
public:
  VulkanInstance();
  ~VulkanInstance();

  operator VkInstance() { return _instance; } 

  void initialize();

  void enable_debug();

  std::shared_ptr<VulkanDevice> create_device(const std::string &dev = "");

private:

  VkInstance _instance = nullptr;
};

#endif