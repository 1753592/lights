#include "VulkanInstance.h"

#include <Windows.h>

#include <vector>
#include <string>
#include <iostream>

#include <vulkan/vulkan.h> 
#include <vulkan/vulkan_win32.h>

#include "VulkanDebug.h"
#include "VulkanDevice.h"


VulkanInstance::VulkanInstance() 
{
  initialize();
}

VulkanInstance::~VulkanInstance() 
{}

void VulkanInstance::enable_debug()
{
  if (!_instance)
    return;

  vks::debug::setupDebugging(_instance);
}

void VulkanInstance::create_device(const std::string &dev) 
{
  uint32_t gpuCount = 0;
  if(vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr) != VK_SUCCESS) {
    throw std::runtime_error("Could not find any gpu.");
  }
  if (gpuCount == 0) {
    throw std::runtime_error("No device with Vulkan support found.");
  }

  std::vector<VkPhysicalDevice> physicalDevices(gpuCount);
  if(vkEnumeratePhysicalDevices(_instance, &gpuCount, physicalDevices.data()) != VK_SUCCESS) {
    throw std::runtime_error("Could not enumerate physical devices.");
  }

  uint32_t selectedDevice = 0;
  VkPhysicalDeviceProperties deviceProperties;
  if (!dev.empty()) {
    for (int i = 0; i < gpuCount; i++) {
      vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
    }
  }

  auto &phyDev = physicalDevices[selectedDevice];
  vkGetPhysicalDeviceProperties(phyDev, &deviceProperties);
  //vkGetPhysicalDeviceFeatures(phyDev, &deviceFeatures);
  //vkGetPhysicalDeviceMemoryProperties(phyDev, &deviceMemoryProperties);

  auto dev = std::make_shared<VulkanDevice>(phyDev);
  dev->create();

}

void VulkanInstance::initialize() 
{
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "demo";
  appInfo.pEngineName = "demo";
  appInfo.apiVersion = VK_API_VERSION_1_3;

  std::vector<const char*> instanceExtensions = {VK_KHR_SURFACE_EXTENSION_NAME};
  instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

  uint32_t extCount = 0;
  std::vector<std::string> supportedExt;
  vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
      for (auto ext : extensions)
        supportedExt.emplace_back(ext.extensionName);
    }
  }
  VkInstanceCreateInfo instanceCreateInfo = {};
  instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instanceCreateInfo.pNext = NULL;
  instanceCreateInfo.pApplicationInfo = &appInfo;

  instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  if (instanceExtensions.size() > 0) {
    instanceCreateInfo.enabledExtensionCount = instanceExtensions.size();
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
  }

  const char* validationLayerName = "VK_LAYER_KHRONOS_validation";
  uint32_t instanceLayerCount;
  vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
  std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
  vkEnumerateInstanceLayerProperties(&instanceLayerCount, instanceLayerProperties.data());

  bool validationLayerPresent = false;
  for (VkLayerProperties layer : instanceLayerProperties) {
    if (strcmp(layer.layerName, validationLayerName) == 0) {
      validationLayerPresent = true;
      break;
    }
  }
  if (validationLayerPresent) {
    instanceCreateInfo.ppEnabledLayerNames = &validationLayerName;
    instanceCreateInfo.enabledLayerCount = 1;
  } else {
    std::cerr << "Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled";
  }

  auto result = vkCreateInstance(&instanceCreateInfo, nullptr, &_instance);

  if (std::find(supportedExt.begin(), supportedExt.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedExt.end())
    vks::debugutils::setup(_instance); 
}
