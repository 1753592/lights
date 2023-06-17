/*
 * Vulkan device class
 *
 * Encapsulates a physical Vulkan device and its logical representation
 *
 * Copyright (C) 2016-2023 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK))
// SRS - Enable beta extensions and make VK_KHR_portability_subset visible
#define VK_ENABLE_BETA_EXTENSIONS
#endif

#include "VulkanDevice.h"
#include "VulkanTools.h"
#include "VulkanBuffer.h"
#include "VulkanInitializers.hpp"
#include <unordered_set>
#include <iostream>
#include <stdexcept>

/**
 * Default constructor
 *
 * @param _physical_device Physical device that is to be used
 */
VulkanDevice::VulkanDevice(VkPhysicalDevice _physical_device)
{
  assert(_physical_device);
  this->_physical_device = _physical_device;

  // Store Properties features, limits and properties of the physical device for later use
  // Device properties also contain limits and sparse properties
  vkGetPhysicalDeviceProperties(_physical_device, &properties);
  // Features should be checked by the examples before using them
  vkGetPhysicalDeviceFeatures(_physical_device, &features);
  // Memory properties are used regularly for creating all kinds of buffers
  vkGetPhysicalDeviceMemoryProperties(_physical_device, &memoryProperties);
  // Queue family properties, used for setting up requested queues upon device creation
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queueFamilyCount, nullptr);
  assert(queueFamilyCount > 0);
  _queueFamilyProperties.resize(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(_physical_device, &queueFamilyCount, _queueFamilyProperties.data());

  // Get list of supported extensions
  uint32_t extCount = 0;
  vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (vkEnumerateDeviceExtensionProperties(_physical_device, nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
      for (auto ext : extensions) {
        supportedExtensions.push_back(ext.extensionName);
      }
    }
  }
}

/**
 * Default destructor
 *
 * @note Frees the logical device
 */
VulkanDevice::~VulkanDevice()
{
  if (_pipe_cache)
    vkDestroyPipelineCache(_logical_device, _pipe_cache, nullptr);

  if (_command_pool) {
    vkDestroyCommandPool(_logical_device, _command_pool, nullptr);
  }
  if (_logical_device) {
    vkDestroyDevice(_logical_device, nullptr);
  }
}

/**
 * Create the logical device based on the assigned physical device, also gets default queue family indices
 *
 * @param enabledFeatures Can be used to enable certain features upon device creation
 * @param pNextChain Optional chain of pointer to extension structures
 * @param useSwapChain Set to false for headless rendering to omit the swapchain device extensions
 * @param requestedQueueTypes Bit flags specifying the queue types to be requested from the device
 *
 * @return VkResult of the device creation call
 */
VkResult VulkanDevice::realize(VkPhysicalDeviceFeatures enabledFeatures, std::vector<const char *> enabledExtensions, void *pNextChain,
                                           bool useSwapChain, VkQueueFlags requestedQueueTypes)
{
  // Desired queues need to be requested upon logical device creation
  // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
  // requests different queue types

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

  // Get queue family indices for the requested queue family types
  // Note that the indices may overlap depending on the implementation

  const float defaultQueuePriority(0.0f);

  // Graphics queue
  if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
    queueFamilyIndices.graphics = queue_family_index(VK_QUEUE_GRAPHICS_BIT);
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &defaultQueuePriority;
    queueCreateInfos.push_back(queueInfo);
  } else {
    queueFamilyIndices.graphics = 0;
  }

  // Dedicated compute queue
  if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
    queueFamilyIndices.compute = queue_family_index(VK_QUEUE_COMPUTE_BIT);
    if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
      // If compute family index differs, we need an additional queue create info for the compute queue
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
  } else {
    // Else we use the same queue
    queueFamilyIndices.compute = queueFamilyIndices.graphics;
  }

  // Dedicated transfer queue
  if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
    queueFamilyIndices.transfer = queue_family_index(VK_QUEUE_TRANSFER_BIT);
    if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
      // If transfer family index differs, we need an additional queue create info for the transfer queue
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
    }
  } else {
    // Else we use the same queue
    queueFamilyIndices.transfer = queueFamilyIndices.graphics;
  }

  // Create the logical device representation
  std::vector<const char *> deviceExtensions(enabledExtensions);
  if (useSwapChain) {
    // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
  ;
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

  // If a pNext(Chain) has been passed, we need to add it to the device creation info
  VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
  if (pNextChain) {
    physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    physicalDeviceFeatures2.features = enabledFeatures;
    physicalDeviceFeatures2.pNext = pNextChain;
    deviceCreateInfo.pEnabledFeatures = nullptr;
    deviceCreateInfo.pNext = &physicalDeviceFeatures2;
  }

#if (defined(VK_USE_PLATFORM_IOS_MVK) || defined(VK_USE_PLATFORM_MACOS_MVK)) && defined(VK_KHR_portability_subset)
  // SRS - When running on iOS/macOS with MoltenVK and VK_KHR_portability_subset is defined and supported by the device, enable the extension
  if (extensionSupported(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME)) {
    deviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
  }
#endif

  if (deviceExtensions.size() > 0) {
    for (const char *enabledExtension : deviceExtensions) {
      if (!extensionSupported(enabledExtension)) {
        std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
      }
    }

    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  }

  this->enabledFeatures = enabledFeatures;

  VkResult result = vkCreateDevice(_physical_device, &deviceCreateInfo, nullptr, &_logical_device);
  if (result != VK_SUCCESS) {
    return result;
  }

  // Create a default command pool for graphics command buffers
  _command_pool = createCommandPool(queueFamilyIndices.graphics);

  vkGetPhysicalDeviceProperties(_physical_device, &properties);
  vkGetPhysicalDeviceFeatures(_physical_device, &features);
  vkGetPhysicalDeviceMemoryProperties(_physical_device, &memoryProperties);

  return result;
}

namespace {
std::string read_file(const std::string &file)
{
  std::string ret;
  auto f = fopen(file.c_str(), "rb");
  if (!f)
    return ret;
  fseek(f, 0, SEEK_END);
  uint64_t len = ftell(f);
  fseek(f, 0, SEEK_SET);
  ret.resize(len, 0);
  fread(&ret[0], 1, len, f);
  fclose(f);
  return ret;
}
}  // namespace

VkShaderModule VulkanDevice::create_shader(const std::string &file)
{
  auto shader_source = read_file(file);
  assert(!shader_source.empty());
  VkShaderModuleCreateInfo create_info{};
  create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  create_info.codeSize = shader_source.size();
  create_info.pCode = (uint32_t *)shader_source.data();

  VkShaderModule shader_module;
  VK_CHECK_RESULT(vkCreateShaderModule(_logical_device, &create_info, nullptr, &shader_module));
  return shader_module;
}

VkPipelineCache VulkanDevice::get_create_pipecache()
{
  if (!_pipe_cache) {
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VK_CHECK_RESULT(vkCreatePipelineCache(_logical_device, &pipelineCacheCreateInfo, nullptr, &_pipe_cache));
  }
  return _pipe_cache;
}

VkDescriptorPool VulkanDevice::get_create_descriptor_pool()
{
  if (!_descriptor_pool) {
    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
                                         {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
                                         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
                                         {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};
    uint32_t sz = sizeof(pool_sizes) / sizeof(VkDescriptorPoolSize);
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1000 * sz;
    pool_info.poolSizeCount = sz;
    pool_info.pPoolSizes = pool_sizes;
    VK_CHECK_RESULT(vkCreateDescriptorPool(_logical_device, &pool_info, nullptr, &_descriptor_pool));
  }
  return _descriptor_pool;
}

/**
 * Get the index of a queue family that supports the requested queue flags
 * SRS - support VkQueueFlags parameter for requesting multiple flags vs. VkQueueFlagBits for a single flag only
 *
 * @param queueFlags Queue flags to find a queue family index for
 *
 * @return Index of the queue family index that matches the flags
 *
 * @throw Throws an exception if no queue family index could be found that supports the requested flags
 */
uint32_t VulkanDevice::queue_family_index(VkQueueFlags queueFlags) const
{
  // Dedicated queue for compute
  // Try to find a queue family index that supports compute but not graphics
  if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++) {
      if ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)) {
        return i;
      }
    }
  }

  // Dedicated queue for transfer
  // Try to find a queue family index that supports transfer but not graphics and compute
  if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags) {
    for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++) {
      if ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) &&
          ((_queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
        return i;
      }
    }
  }

  // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
  for (uint32_t i = 0; i < static_cast<uint32_t>(_queueFamilyProperties.size()); i++) {
    if ((_queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags) {
      return i;
    }
  }

  throw std::runtime_error("Could not find a matching queue family index");
}

/**
 * Get the index of a memory type that has all the requested property bits set
 *
 * @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
 * @param properties Bit mask of properties for the memory type to request
 * @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found
 *
 * @return Index of the requested memory type
 *
 * @throw Throws an exception if memTypeFound is null and no memory type could be found that supports the requested properties
 */

std::optional<uint32_t> VulkanDevice::memory_type_index(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
    if ((typeBits & 1)) {
      if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }
    typeBits >>= 1;
  }
  return std::nullopt;
}

/**
 * Create a buffer on the device
 *
 * @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
 * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
 * @param size Size of the buffer in byes
 * @param buffer Pointer to the buffer handle acquired by the function
 * @param memory Pointer to the memory handle acquired by the function
 * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
 *
 * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
 */
std::shared_ptr<VulkanBuffer> VulkanDevice::create_buffer(VkBufferUsageFlags usageFlags, 
  VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, void *data)
{
  auto buffer = std::make_shared<VulkanBuffer>(shared_from_this());

  VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  VK_CHECK_RESULT(vkCreateBuffer(_logical_device, &bufferCreateInfo, nullptr, &buffer->_buffer));

  VkMemoryRequirements memReqs;
  VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
  vkGetBufferMemoryRequirements(_logical_device, *buffer, &memReqs);
  memAlloc.allocationSize = memReqs.size;

  auto index = memory_type_index(memReqs.memoryTypeBits, memoryPropertyFlags);
  if (!index) return nullptr;

  memAlloc.memoryTypeIndex = *index;
  VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
  if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
    allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
    allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
    memAlloc.pNext = &allocFlagsInfo;
  }

  VkDeviceMemory memory;
  VK_CHECK_RESULT(vkAllocateMemory(_logical_device, &memAlloc, nullptr, &memory));
  buffer->_memory = memory;

  if (data != nullptr) {
    void *mapped;
    VK_CHECK_RESULT(vkMapMemory(_logical_device, memory, 0, size, 0, &mapped));
    memcpy(mapped, data, size);
    // If host coherency hasn't been requested, do a manual flush to make writes visible
    if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) {
      VkMappedMemoryRange mappedRange = vks::initializers::mappedMemoryRange();
      mappedRange.memory = memory;
      mappedRange.offset = 0;
      mappedRange.size = size;
      vkFlushMappedMemoryRanges(_logical_device, 1, &mappedRange);
    }
    vkUnmapMemory(_logical_device, memory);
  }

  VK_CHECK_RESULT(vkBindBufferMemory(_logical_device, buffer->_buffer, memory, 0));
  buffer->_size = size;

  return buffer;
}

/**
 * Copy buffer data from src to dst using VkCmdCopyBuffer
 *
 * @param src Pointer to the source buffer to copy from
 * @param dst Pointer to the destination buffer to copy to
 * @param queue Pointer
 * @param copyRegion (Optional) Pointer to a copy region, if NULL, the whole buffer is copied
 *
 * @note Source and destination pointers must have the appropriate transfer usage flags set (TRANSFER_SRC / TRANSFER_DST)
 */
void VulkanDevice::copy_buffer(VulkanBuffer *src, VulkanBuffer *dst, VkQueue queue, VkBufferCopy *copyRegion)
{
  assert(dst->_size <= src->_size);
  assert(src->_buffer && dst->_buffer);
  VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
  VkBufferCopy bufferCopy{};
  if (copyRegion == nullptr) {
    bufferCopy.size = src->_size;
  } else {
    bufferCopy = *copyRegion;
  }

  vkCmdCopyBuffer(copyCmd, src->_buffer, dst->_buffer, 1, &bufferCopy);

  flushCommandBuffer(copyCmd, queue);
}

/**
 * Create a command pool for allocation command buffers from
 *
 * @param queueFamilyIndex Family index of the queue to create the command pool for
 * @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
 *
 * @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
 *
 * @return A handle to the created command buffer
 */
VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
{
  VkCommandPoolCreateInfo cmdPoolInfo = {};
  cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
  cmdPoolInfo.flags = createFlags;
  VkCommandPool cmdPool;
  vkCreateCommandPool(_logical_device, &cmdPoolInfo, nullptr, &cmdPool);
  return cmdPool;
}

/**
 * Allocate a command buffer from the command pool
 *
 * @param level Level of the new command buffer (primary or secondary)
 * @param pool Command pool from which the command buffer will be allocated
 * @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
 *
 * @return A handle to the allocated command buffer
 */
VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
{
  VkCommandBuffer cmdBuffer;
  VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(pool, level, 1);
  VK_CHECK_RESULT(vkAllocateCommandBuffers(_logical_device, &cmdBufAllocateInfo, &cmdBuffer));
  // If requested, also start recording for the new command buffer
  if (begin) {
    VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
    VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
  }
  return cmdBuffer;
}

VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin)
{
  return createCommandBuffer(level, _command_pool, begin);
}

/**
 * Finish command buffer recording and submit it to a queue
 *
 * @param commandBuffer Command buffer to flush
 * @param queue Queue to submit the command buffer to
 * @param pool Command pool on which the command buffer has been created
 * @param free (Optional) Free the command buffer once it has been submitted (Defaults to true)
 *
 * @note The queue that the command buffer is submitted to must be from the same family index as the pool it was allocated from
 * @note Uses a fence to ensure command buffer has finished executing
 */
void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
{
  if (commandBuffer == VK_NULL_HANDLE) {
    return;
  }

  VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

  VkSubmitInfo submitInfo = vks::initializers::submitInfo();
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;
  // Create fence to ensure that the command buffer has finished executing
  VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
  VkFence fence;
  VK_CHECK_RESULT(vkCreateFence(_logical_device, &fenceInfo, nullptr, &fence));
  // Submit to the queue
  VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence));
  // Wait for the fence to signal that command buffer has finished executing
  VK_CHECK_RESULT(vkWaitForFences(_logical_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
  vkDestroyFence(_logical_device, fence, nullptr);
  if (free) {
    vkFreeCommandBuffers(_logical_device, pool, 1, &commandBuffer);
  }
}

void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
{
  return flushCommandBuffer(commandBuffer, queue, _command_pool, free);
}

std::vector<VkCommandBuffer> VulkanDevice::createCommandBuffers(uint32_t n)
{
  std::vector<VkCommandBuffer> cmd_bufs(n);
  VkCommandBufferAllocateInfo cmdBufAllocateInfo =
      vks::initializers::commandBufferAllocateInfo(_command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, n);

  VK_CHECK_RESULT(vkAllocateCommandBuffers(_logical_device, &cmdBufAllocateInfo, cmd_bufs.data()));
  return cmd_bufs;
}

void VulkanDevice::destroyCommandBuffers(std::vector<VkCommandBuffer> &cmdbufs)
{
  vkFreeCommandBuffers(_logical_device, _command_pool, static_cast<uint32_t>(cmdbufs.size()), cmdbufs.data());
  cmdbufs.clear();
}

std::vector<VkFence> VulkanDevice::createFences(uint32_t n)
{
  std::vector<VkFence> fences;
  VkFenceCreateInfo fenceCreateInfo = vks::initializers::fenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
  fences.resize(n);
  for (auto &fence : fences) {
    VK_CHECK_RESULT(vkCreateFence(_logical_device, &fenceCreateInfo, nullptr, &fence));
  }
  return fences;
}

VkQueue VulkanDevice::graphic_queue(uint32_t idx)
{
  VkQueue queue = VK_NULL_HANDLE;
  vkGetDeviceQueue(_logical_device, queueFamilyIndices.graphics, idx, &queue);
  return queue;
}

/**
 * Check if an extension is supported by the (physical device)
 *
 * @param extension Name of the extension to check
 *
 * @return True if the extension is supported (present in the list read at device creation time)
 */
bool VulkanDevice::extensionSupported(std::string extension)
{
  return (std::find(supportedExtensions.begin(), supportedExtensions.end(), extension) != supportedExtensions.end());
}

/**
 * Select the best-fit depth format for this device from a list of possible depth (and stencil) formats
 *
 * @param checkSamplingSupport Check if the format can be sampled from (e.g. for shader reads)
 *
 * @return The depth format that best fits for the current device
 *
 * @throw Throws an exception if no depth format fits the requirements
 */
VkFormat VulkanDevice::getSupportedDepthFormat(bool checkSamplingSupport)
{
  // All depth formats may be optional, so we need to find a suitable depth format to use
  std::vector<VkFormat> depthFormats = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
                                        VK_FORMAT_D16_UNORM};
  for (auto &format : depthFormats) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(_physical_device, format, &formatProperties);
    // Format must support depth stencil attachment for optimal tiling
    if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
      if (checkSamplingSupport) {
        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)) {
          continue;
        }
      }
      return format;
    }
  }
  throw std::runtime_error("Could not find a matching depth format");
}
