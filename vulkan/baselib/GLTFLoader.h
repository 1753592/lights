#pragma once

#include <string>
#include <memory>
#include <vulkan/vulkan_core.h>

#include "tvec.h"

namespace tinygltf{
  class Model;
  class Accessor;
  class BufferView;
  class Primitive;
} 

class VulkanDevice;

class MeshPrimitive;

class GLTFLoader{
public:
  GLTFLoader(std::shared_ptr<VulkanDevice> &dev);
  ~GLTFLoader();

  bool load_file(const std::string &file);

private:
  void create_pipeline();

  std::shared_ptr<MeshPrimitive> create_primitive(const tinygltf::Primitive *pri);

  VkFormat attr_format(const tinygltf::Accessor *acc);

private:
  std::shared_ptr<VulkanDevice> _dev;
  std::shared_ptr<tinygltf::Model> _m;
};
