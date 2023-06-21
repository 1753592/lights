#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL_events.h>

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

  bool mouse_down(SDL_MouseButtonEvent &ev);

  bool mouse_up(SDL_MouseButtonEvent &ev);

  bool mouse_move(SDL_MouseMotionEvent &ev);

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

  std::shared_ptr<VulkanBuffer> _vert_buf, _vert_buf_bak;
  std::shared_ptr<VulkanBuffer> _index_buf, _index_buf_bak;

  VkImage _font_img;
  VkDeviceMemory _font_memory;
  VkImageView _font_view;
};