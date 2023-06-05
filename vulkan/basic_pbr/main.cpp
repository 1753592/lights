#include <exception>
#include <stdexcept>

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "VulkanDebug.h"
#include "VulkanView.h"
#include "VulkanInstance.h"

#define WM_PAINT 1

void loop(SDL_Window *win) 
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
          if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            //recreateSwapChain();
          break;
        case SDL_USEREVENT:
          if (event.user.code == WM_PAINT)
            //drawFrame();
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
          break;
        case SDL_MOUSEWHEEL: {
          //osg::Vec3 tmp = _cam - _pos;
          //tmp *= event.wheel.y;
          //_cam += tmp * 0.1;
          break;
        }
        default:
          break;
      }
    }
  }
}


int main(int argc, char** argv)
{
  SDL_Window* win = 0;
  try {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      throw std::runtime_error("sdl init error.");
    win = SDL_CreateWindow("demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (win == nullptr)
      throw std::runtime_error("could not create sdl window.");

    VulkanInstance inst;
    inst.enable_debug();
    auto dev = inst.create_device();

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(win, inst, &surface))
      throw std::runtime_error("could not create vk surface.");

  } catch (std::runtime_error& e) {
    printf("%s", e.what());
    return -1;
  }
  loop(win);
  return 0;
}