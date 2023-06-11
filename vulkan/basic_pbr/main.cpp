#include <exception>
#include <stdexcept>

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "VulkanDebug.h"
#include "VulkanView.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanTools.h"

#include "SimpleShape.h"

#define WM_PAINT 1

VulkanInstance inst;

class Test{
public:
  Test(const std::shared_ptr<VulkanDevice> &dev) 
    : _device(dev)
  {
    _swapchain = std::make_shared<VulkanSwapChain>(dev); 
    _queue = dev->getGraphicQueue(0);

    create_shpere();
  }

  ~Test()
  {
    vkDeviceWaitIdle(*_device);

    free_resource();
    for (auto fence : _fences)
      vkDestroyFence(*_device, fence, nullptr);

    _device->destroyCommandBuffers(_cmd_bufs);

    vkDestroyRenderPass(*_device, _render_pass, nullptr);
    
    vkDestroySemaphore(*_device, _presentSemaphore, nullptr);
    vkDestroySemaphore(*_device, _renderSemaphore, nullptr);

    auto surface = _swapchain->surface();
    _swapchain.reset();

    if(surface) {
      vkDestroySurfaceKHR(inst, surface, nullptr);
    }
  }

  void set_window(SDL_Window *win) 
  {
    SDL_GetWindowSize(win, &_w, &_h);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(win, inst, &surface))
      throw std::runtime_error("could not create vk surface.");

    _swapchain->set_surface(surface);
    _swapchain->realize(_w, _h, true);

    create_sync_object();

    int count = _swapchain->image_count();

    _render_pass = _swapchain->create_render_pass();
    
    _cmd_bufs = _device->createCommandBuffers(count);

    apply_resource();

  }

  void apply_resource() 
  {
    int count = _swapchain->image_count();
    _depth = _swapchain->create_depth(_w, _h);
    _frame_bufs = _swapchain->create_frame_buffer(_render_pass, _depth);

    build_command_buffers(_frame_bufs, _render_pass);

    if(_fences.size() != _cmd_bufs.size()) {
      for (auto fence : _fences)
        vkDestroyFence(*_device, fence, nullptr);
      _fences = _device->createFences(_cmd_bufs.size()); 
    }
  }

  void free_resource()
  {
    for (auto &framebuf : _frame_bufs)
      vkDestroyFramebuffer(*_device, framebuf, nullptr);
    _frame_bufs.clear();

    vkDestroyImageView(*_device, _depth.view, nullptr);
    vkDestroyImage(*_device, _depth.image, nullptr);
    vkFreeMemory(*_device, _depth.mem, nullptr);
    _depth = {VK_NULL_HANDLE};
  }

  void create_sync_object() 
  {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = nullptr;

    // Semaphore used to ensure that image presentation is complete before starting to submit again
    VK_CHECK_RESULT(vkCreateSemaphore(*_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));

    // Semaphore used to ensure that all commands submitted have been finished before submitting the image to the queue
    VK_CHECK_RESULT(vkCreateSemaphore(*_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));
  }

  void update() 
  {
    SDL_Event ev;
    ev.type = SDL_USEREVENT;
    ev.user.code = WM_PAINT;
    SDL_PushEvent(&ev);
  }

  void draw() 
  { 
    //vkWaitForFences(*_device, 1, _fences[]);
    auto [result, index] = _swapchain->acquire_image(_presentSemaphore); 
    if (!((result == VK_SUCCESS) || (result == VK_SUBOPTIMAL_KHR))) {
      VK_CHECK_RESULT(result);
    }
  
    VK_CHECK_RESULT(vkWaitForFences(*_device, 1, &_fences[index], VK_TRUE, UINT64_MAX));
    VK_CHECK_RESULT(vkResetFences(*_device, 1, &_fences[index]));

    VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pWaitDstStageMask = &waitStageMask;                // Pointer to the list of pipeline stages that the semaphore waits will occur at
    submitInfo.waitSemaphoreCount = 1;                            // One wait semaphore
    submitInfo.signalSemaphoreCount = 1;                          // One signal semaphore
    submitInfo.pCommandBuffers = &_cmd_bufs[index];               // Command buffers(s) to execute in this batch (submission)
    submitInfo.commandBufferCount = 1;                            // One command buffer

		submitInfo.pWaitSemaphores = &_presentSemaphore;      // Semaphore(s) to wait upon before the submitted command buffer starts executing
		submitInfo.pSignalSemaphores = &_renderSemaphore;     // Semaphore(s) to be signaled when command buffers have completed

    // Submit to the graphics queue passing a wait fence
    VK_CHECK_RESULT(vkQueueSubmit(_queue, 1, &submitInfo, _fences[index]));

    {
      _frame = index;
      auto present = _swapchain->queue_present(_queue, _frame, _renderSemaphore);
      if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR))) {
        VK_CHECK_RESULT(present);
      }
    }
  }

  void loop()
  {
    bool running = true;
    while (running) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch (event.type) {
          case SDL_QUIT:
            running = false;
            break;
          case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
              vkDeviceWaitIdle(*_device);
              _w = event.window.data1;
              _h = event.window.data2;
              _swapchain->realize(_w, _h, true);
              free_resource();
              apply_resource();
            }
            break;
          case SDL_USEREVENT:
            if (event.user.code == WM_PAINT)
              draw();
              break;
          case SDL_MOUSEMOTION:
            if (event.motion.state & SDL_BUTTON_LMASK) {
              //  tg::vec3 tmp = _cam - _pos;
              //  float xrad = event.motion.xrel / 100.f;
              //  float yrad = event.motion.yrel / 100.f;
              //  osg::Matrixf m;
              //  m.makeRotate(-xrad, _up);
              //  tmp = m.preMult(tmp);
              //  osg::Vec3 rt = _up ^ tmp;
              //  m.makeRotate(-yrad, rt);
              //  tmp = m.preMult(tmp);
              //  _up = m.preMult(_up);
              //  _cam = _pos + tmp;
              //} else if (event.motion.state & SDL_BUTTON_RMASK) {
              //  osg::Vec3 tmp = _cam - _pos;
              //  tmp.normalize();
              //  osg::Vec3 rt = _up ^ tmp;
              //  rt *= -event.motion.xrel * 0.1;
              //  auto up = _up * event.motion.yrel * 0.1;
              //  _cam += rt;
              //  _cam += up;
              //  _pos += rt;
              //  _pos += up;
            } else if (event.motion.state & SDL_BUTTON_MMASK) {
            }
            update();
            break;
          case SDL_MOUSEWHEEL: {
            // osg::Vec3 tmp = _cam - _pos;
            // tmp *= event.wheel.y;
            //_cam += tmp * 0.1;
            update();
            break;
          }
          default:
            break;
        }
      }
    }
  }

  void build_command_buffers(std::vector<VkFramebuffer> &framebuffers, VkRenderPass renderPass)
  {
    assert(framebuffers.size() == _cmd_bufs.size());

    VkCommandBufferBeginInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buf_info.pNext = nullptr;

    VkClearValue clearValues[2];
    clearValues[0].color = {{0.0, 0.0, 0.2, 1.0}};
    clearValues[1].depthStencil = {1, 0};

    
		VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = _w;
    renderPassBeginInfo.renderArea.extent.height = _h;
    renderPassBeginInfo.clearValueCount = 2;
    renderPassBeginInfo.pClearValues = clearValues;

    for(int i = 0; i < _cmd_bufs.size(); i++) {
      renderPassBeginInfo.framebuffer = framebuffers[i];
      VK_CHECK_RESULT(vkBeginCommandBuffer(_cmd_bufs[i], &buf_info));

      vkCmdBeginRenderPass(_cmd_bufs[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
      vkCmdEndRenderPass(_cmd_bufs[i]);

      VK_CHECK_RESULT(vkEndCommandBuffer(_cmd_bufs[i]));
    }
  }


  void create_shpere() { 
    Sphere sp(vec3(0), 1);
    sp.build();
    auto &verts = sp.get_vertex();
    auto &norms = sp.get_norms();
    auto &uv = sp.get_uvs();
    auto &index = sp.get_index();


  }

private:

  std::shared_ptr<VulkanDevice> _device;
  std::shared_ptr<VulkanSwapChain> _swapchain;

  int _w, _h;
  uint32_t _frame = 0;

  VkRenderPass _render_pass = VK_NULL_HANDLE;
  VulkanSwapChain::DepthUnit _depth;

  VkSemaphore _presentSemaphore;
  VkSemaphore _renderSemaphore;

  VkQueue _queue;

  std::vector<VkCommandBuffer> _cmd_bufs;
  std::vector<VkFramebuffer> _frame_bufs;
  std::vector<VkFence> _fences;
};


int main(int argc, char** argv)
{
  SDL_Window* win = 0;
  std::shared_ptr<Test> test;
  try {
    inst.initialize();

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      throw std::runtime_error("sdl init error.");
    win = SDL_CreateWindow("demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (win == nullptr)
      throw std::runtime_error("could not create sdl window.");

    inst.enable_debug();
    auto dev = inst.create_device();

    test = std::make_shared<Test>(dev);
    test->set_window(win);

  } catch (std::runtime_error& e) {
    printf("%s", e.what());
    return -1;
  }
  test->loop();
  return 0;
}