#pragma once
#include <vulkan/vulkan.h>

struct ImageUnit {
  VkImage image;
  VkImageView view;
  VkDeviceMemory mem;
};
