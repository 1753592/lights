#ifndef __VULKAN_VIEW_INC__
#define __VULKAN_VIEW_INC__

class VulkanDevice;

#include <memory>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanDef.h"
#include "Manipulator.h"

class VulkanSwapChain;
class VulkanImGUI;

class VulkanView {
public:
  VulkanView(const std::shared_ptr<VulkanDevice> &dev, bool overlay = true);

  virtual ~VulkanView();

  void set_surface(VkSurfaceKHR surface, int w, int h);

  void update_overlay();

  void frame(bool continus = true);

  VulkanDevice *device() { return _device.get(); }

  VulkanSwapChain *swapchain() { return _swapchain.get(); }

  VkRenderPass render_pass() { return _render_pass; }

  uint32_t frame_count();

  Manipulator &manipulator() { return _manip; }

  int width() { return _w; }
  int height() { return _h; }

  virtual void resize(int w, int h) = 0;

  virtual void update_scene(){};
  virtual void build_command_buffer(VkCommandBuffer cmd_buf) = 0;

  virtual void wheel(int delta){};
  virtual void left_drag(int x, int y, int xdel, int ydel){};
  virtual void right_drag(int x, int y, int xdel, int ydel){};
  virtual void key_up(int){};

protected:

  void update();

  void rebuild_command();

private:
  void initialize();

  void check_frame();

  void clear_frame();

  void create_sync_objs();

  void build_command_buffers();

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
  ImageUnit _depth = {VK_NULL_HANDLE};

  VkSemaphore _presentSemaphore = VK_NULL_HANDLE;
  VkSemaphore _renderSemaphore = VK_NULL_HANDLE;
  std::vector<VkFence> _fences;

  Manipulator _manip;
};

#endif