#pragma once

#include <vulkan/vulkan.h>

#include <memory>

class VulkanDevice;
class VulkanBuffer;

class VulkanImGUI{
public:
  VulkanImGUI(std::shared_ptr<VulkanDevice> dev);
  ~VulkanImGUI();

  void resize(int w, int h);

  void create_pipeline(VkRenderPass render_pass, VkFormat clrformat, VkFormat depformat);

  bool update();

  void draw(const VkCommandBuffer cmdbuf);

private:
  void create_canvas();

private:
  bool _initialized = false;
  std::shared_ptr<VulkanDevice> _device;

  VkSampler _sampler = VK_NULL_HANDLE;
  VkDescriptorPool _descriptor_pool = VK_NULL_HANDLE;
  VkDescriptorSet _descriptor_set = VK_NULL_HANDLE;

  VkPipelineLayout _pipe_layout = VK_NULL_HANDLE;
  VkPipeline _pipeline = VK_NULL_HANDLE;

  std::shared_ptr<VulkanBuffer> _vert_buf;
  std::shared_ptr<VulkanBuffer> _index_buf;

  VkImage _font_tex;
  VkDeviceMemory _font_memory;
  VkImageView _font_view;
};