#ifndef __VULKAN_VIEW_INC__
#define __VULKAN_VIEW_INC__

class VulkanDevice;

#include <memory>
#include <vector>

#include "vulkan/vulkan.h"
#include "Manipulator.h"

class VulkanSwapChain;
class VulkanImGUI;

class VulkanView {
public:
  VulkanView(const std::shared_ptr<VulkanDevice> &dev);

  virtual ~VulkanView();

  void set_surface(VkSurfaceKHR surface, int w, int h);

  void frame(bool continus = true);

  VulkanDevice *device() { return _device.get(); }

  VkRenderPass render_pass() { return _render_pass; }

  uint32_t frame_count() { return _swapchain->image_count(); }

  Manipulator &manipulator() { return _manip; }

  uint32_t width() { return _w; }
  uint32_t height() { return _h; }

  virtual void resize(int w, int h) = 0;

  virtual void update_scene(){};
  virtual void build_command_buffer(VkCommandBuffer cmd_buf) = 0;

  virtual void wheel(int delta){};
  virtual void left_clicked(int x, int y) {};
  virtual void left_drag(int x, int y, int xdel, int ydel){};
  virtual void right_drag(int x, int y, int xdel, int ydel){};

protected:

  void update();

private:
  void initialize();

  void check_frame();

  void clear_frame();

  void create_sync_objs();

  void build_command_buffers(std::vector<VkFramebuffer> &framebuffers, VkRenderPass renderPass);

  void resize_impl(int w, int h);

  void render();

private:
  std::shared_ptr<VulkanDevice> _device;
  std::shared_ptr<VulkanSwapChain> _swapchain;

  std::shared_ptr<VulkanImGUI> _imgui = 0;

  std::vector<VkCommandBuffer> _cmd_bufs;
  std::vector<VkFramebuffer> _frame_bufs;

  int _w, _h;
  uint32_t _frame = 0;

  VkRenderPass _render_pass = VK_NULL_HANDLE;
  VulkanSwapChain::DepthImage _depth;

  VkSemaphore _presentSemaphore;
  VkSemaphore _renderSemaphore;
  std::vector<VkFence> _fences;

  Manipulator _manip;
};

#endif