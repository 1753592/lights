#pragma once

struct ImageUnit {
  VkImage image;
  VkImageView view;
  VkDeviceMemory mem;
};
