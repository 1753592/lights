/*
 * Vulkan device class
 *
 * Encapsulates a physical Vulkan device and its logical representation
 *
 * Copyright (C) 2016-2023 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "vulkan/vulkan.h"

#include <vector>
#include <string>
#include <algorithm>
#include <assert.h>
#include <exception>

class VulkanBuffer;

class VulkanDevice {
public:

  explicit VulkanDevice(VkPhysicalDevice physicalDevice);
  ~VulkanDevice();
  operator VkDevice() const { return _logical_device; };

  VkResult realize(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, void *pNextChain,
                               bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

  uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;
  uint32_t getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound = nullptr) const;

  VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory,
                        void *data = nullptr);
  VkResult createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VulkanBuffer *buffer, VkDeviceSize size,
                        void *data = nullptr);
  void copyBuffer(VulkanBuffer *src, VulkanBuffer *dst, VkQueue queue, VkBufferCopy *copyRegion = nullptr);

  VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);
  bool extensionSupported(std::string extension);
  VkFormat getSupportedDepthFormat(bool checkSamplingSupport);

private:
  /** @brief Physical device representation */
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
  /** @brief Logical device representation (application's view of the device) */
  VkDevice _logical_device = VK_NULL_HANDLE;
  /** @brief Properties of the physical device including limits that the application can check against */
  VkPhysicalDeviceProperties properties;
  /** @brief Features of the physical device that an application can use to check if a feature is supported */
  VkPhysicalDeviceFeatures features;
  /** @brief Features that have been enabled for use on the physical device */
  VkPhysicalDeviceFeatures enabledFeatures;
  /** @brief Memory types and heaps of the physical device */
  VkPhysicalDeviceMemoryProperties memoryProperties;
  /** @brief Queue family properties of the physical device */
  std::vector<VkQueueFamilyProperties> queueFamilyProperties;
  /** @brief List of extensions supported by the device */
  std::vector<std::string> supportedExtensions;
  /** @brief Default command pool for the graphics queue family index */
  VkCommandPool _command_pool = VK_NULL_HANDLE;
  /** @brief Contains queue family indices */
  struct {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
  } queueFamilyIndices;
};
