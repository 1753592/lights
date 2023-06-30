#include <exception>
#include <stdexcept>

#include "Manipulator.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "VulkanDebug.h"
#include "VulkanView.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"

#include "SimpleShape.h"

#include "VulkanView.h"

#define WM_PAINT 1

constexpr float fov = 60;


VulkanInstance &inst = VulkanInstance::instance();

struct {
  tg::mat4 prj;
  tg::mat4 view;
  tg::mat4 model;
  tg::vec4 cam;
} matrix_ubo;

struct{
  struct alignas(16) aligned_vec3 : vec3 {};
  aligned_vec3 light_pos[4] = {vec3(10, -10, 10), vec3(-10, -10, 10), vec3(-10, -10, -10), vec3(10, -10, -10)};
  aligned_vec3 light_color[4] = {vec3(300), vec3(300), vec3(300), vec3(300)};
} lights_ubo;

struct alignas(16){
  struct alignas(16) aligned_vec3 : vec3 {};

	float metallic;
	float roughness;
	float ao;
	aligned_vec3 albedo;
} material_ubo;

class Test : public VulkanView{
public:
  Test(const std::shared_ptr<VulkanDevice> &dev) : VulkanView(dev)
  {
  }

  ~Test()
  {
  }

  void set_window(SDL_Window *win)
  {
    int w = 0, h = 0;
    SDL_GetWindowSize(win, &w, &h);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(win, inst, &surface))
      throw std::runtime_error("could not create vk surface.");

    set_surface(surface, w, h);
  }

  void resize(int, int){}
  void build_command_buffer(VkCommandBuffer cmd_buf) {}

  void update_ubo()
  {
    auto &manip = manipulator();
    matrix_ubo.cam = manip.eye();
    matrix_ubo.view = manip.view_matrix();
    matrix_ubo.model = tg::mat4::identity();
    matrix_ubo.prj = tg::perspective<float>(fov, float(width()) / height(), 0.1, 1000);
    // tg::near_clip(matrix_ubo.prj, tg::vec4(0, 0, -1, 0.5));
    auto dev = device();
    uint8_t *data = 0;
    VK_CHECK_RESULT(vkMapMemory(*dev, _ubo_buf->memory(), 0, sizeof(matrix_ubo), 0, (void **)&data));
    memcpy(data, &matrix_ubo, sizeof(matrix_ubo));
    vkUnmapMemory(*dev, _ubo_buf->memory());
  }

private:
  std::shared_ptr<VulkanBuffer> _ubo_buf;

};


int main(int argc, char** argv)
{
  SDL_Window* win = 0;
  std::shared_ptr<Test> test;
  try {
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
  test->frame();
  return 0;
}