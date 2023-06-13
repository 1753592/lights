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
#include <optional>
#include <memory>

class VulkanBuffer;

class VulkanDevice {
public:

  explicit VulkanDevice(VkPhysicalDevice _physical_device);
  ~VulkanDevice();
  operator VkDevice() const { return _logical_device; };
  VkPhysicalDevice physical_device() { return _physical_device; }
  VkCommandPool command_pool() { return _command_pool; }

  VkResult realize(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, void *pNextChain,
                               bool useSwapChain = true, VkQueueFlags requestedQueueTypes = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT);

  VkShaderModule create_shader(const std::string &file);

  VkPipelineCache get_create_pipecache();
    
  uint32_t getQueueFamilyIndex(VkQueueFlags queueFlags) const;

  std::optional<uint32_t> getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

  std::shared_ptr<VulkanBuffer> createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, 
    VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data = nullptr);
  std::shared_ptr<VulkanBuffer> createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, 
    VkDeviceSize size, void *data = nullptr);
  void copyBuffer(VulkanBuffer *src, VulkanBuffer *dst, VkQueue queue, VkBufferCopy *copyRegion = nullptr);

  VkCommandPool createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin = false);
  VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, bool begin = false);
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free = true);
  void flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free = true);

  std::vector<VkCommandBuffer> createCommandBuffers(uint32_t n);
  void destroyCommandBuffers(std::vector<VkCommandBuffer> &cmdbufs);

  std::vector<VkFence> createFences(uint32_t n);

  VkQueue getGraphicQueue(uint32_t idx);

  bool extensionSupported(std::string extension);
  VkFormat getSupportedDepthFormat(bool checkSamplingSupport);

public:

  const std::vector<VkQueueFamilyProperties> &queueFamilyProperties() { return _queueFamilyProperties; }

private:
  /** @brief Physical device representation */
  VkPhysicalDevice _physical_device = VK_NULL_HANDLE;
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
  std::vector<VkQueueFamilyProperties> _queueFamilyProperties;
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

  VkPipelineCache _pipe_cache = VK_NULL_HANDLE;
};
