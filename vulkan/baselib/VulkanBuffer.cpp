#include "VulkanBuffer.h"


VulkanBuffer::VulkanBuffer()
{
}

VulkanBuffer::~VulkanBuffer()
{
}

/**
 * Setup the default descriptor for this _buffer
 *
 * @param size (Optional) Size of the _memory range of the descriptor
 * @param offset (Optional) Byte offset from beginning
 *
 */
void VulkanBuffer::setupDescriptor(VkDeviceSize size, VkDeviceSize offset)
{
  _descriptor.offset = offset;
  _descriptor.buffer = _buffer;
  _descriptor.range = size;
}

/**
 * Flush a _memory range of the _buffer to make it visible to the _device
 *
 * @note Only required for non-coherent _memory
 *
 * @param size (Optional) Size of the _memory range to flush. Pass VK_WHOLE_SIZE to flush the complete _buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the flush call
 */
VkResult VulkanBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = _memory;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkFlushMappedMemoryRanges(_device, 1, &mappedRange);
}

/**
 * Invalidate a _memory range of the _buffer to make it visible to the host
 *
 * @note Only required for non-coherent _memory
 *
 * @param size (Optional) Size of the _memory range to invalidate. Pass VK_WHOLE_SIZE to invalidate the complete _buffer range.
 * @param offset (Optional) Byte offset from beginning
 *
 * @return VkResult of the invalidate call
 */
VkResult VulkanBuffer::invalidate(VkDeviceSize size, VkDeviceSize offset)
{
  VkMappedMemoryRange mappedRange = {};
  mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
  mappedRange.memory = _memory;
  mappedRange.offset = offset;
  mappedRange.size = size;
  return vkInvalidateMappedMemoryRanges(_device, 1, &mappedRange);
}

/**
 * Release all Vulkan resources held by this _buffer
 */
void VulkanBuffer::destroy()
{
  if (_buffer) {
    vkDestroyBuffer(_device, _buffer, nullptr);
  }
  if (_memory) {
    vkFreeMemory(_device, _memory, nullptr);
  }
}