#include "HUDPass.h"
#include "VulkanInitializers.hpp"
#include "VulkanTools.h"

HUDPass::HUDPass(const std::shared_ptr<VulkanDevice> &dev) : VulkanPass(dev)
{
}

HUDPass::~HUDPass()
{
}

void HUDPass::initialize()
{
  VkRenderPass render_pass = VK_NULL_HANDLE;

  VkAttachmentDescription attachment = {};
  attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
  attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
  attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorReference = {};
  colorReference.attachment = 0;
  colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {0};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &colorReference;

  VkSubpassDependency dependency;
  dependency.srcSubpass = 0;
  dependency.dstSubpass = VK_SUBPASS_EXTERNAL;
  dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo renderPassCreateInfo = vks::initializers::renderPassCreateInfo();
  renderPassCreateInfo.attachmentCount = 1;
  renderPassCreateInfo.pAttachments = &attachment;
  renderPassCreateInfo.subpassCount = 1;
  renderPassCreateInfo.pSubpasses = &subpass;
  renderPassCreateInfo.dependencyCount = 1;
  renderPassCreateInfo.pDependencies = &dependency;

  VkRenderPass vkpass = VK_NULL_HANDLE;
  VK_CHECK_RESULT(vkCreateRenderPass(*_device, &renderPassCreateInfo, nullptr, &vkpass));
  _render_pass = vkpass;
}
