#include "MeshInstance.h"

#include "VulkanTools.h"
#include "VulkanDevice.h"

MeshInstance::MeshInstance(std::shared_ptr<VulkanDevice> &dev) : _device(dev)
{
}

MeshInstance::~MeshInstance()
{
  if (_pipe_layout)
    vkDestroyPipelineLayout(*_device, _pipe_layout, nullptr);
}

void MeshInstance::add_primitive(std::shared_ptr<MeshPrimitive>& pri) {
  _pris.emplace_back(pri);
}

