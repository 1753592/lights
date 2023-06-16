#include "VulkanImGUI.h"

#include "VulkanDevice.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"

#include "VulkanTools.h"


VulkanImGUI::VulkanImGUI(std::shared_ptr<VulkanDevice> dev, VkSurfaceKHR surface) : _device(dev)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  auto &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
  ImGui::StyleColorsLight();

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = 0;
  init_info.PhysicalDevice = dev->physical_device();
  init_info.Device = *dev;
  init_info.QueueFamily = dev->queue_family_index(VK_QUEUE_GRAPHICS_BIT);
  init_info.Queue = dev->graphic_queue();
  init_info.PipelineCache = dev->get_create_pipecache();
  init_info.DescriptorPool = dev->get_create_descriptor_pool();
  init_info.Subpass = 0;
  init_info.MinImageCount = 0;
  init_info.ImageCount = 0;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = nullptr;
  init_info.CheckVkResultFn = [](VkResult err) { VK_CHECK_RESULT(err) };
  ImGui_ImplVulkan_Init(&init_info, 0);
}

VulkanImGUI::~VulkanImGUI()
{
}
