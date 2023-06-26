#include <exception>
#include <stdexcept>

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

#include "imgui/imgui.h"

#define WM_PAINT 1

constexpr float fov = 60;

VulkanInstance &inst = VulkanInstance::instance();

struct {
  tg::mat4 prj;
  tg::mat4 view;
  tg::mat4 model;
  tg::vec4 cam;
} matrix_ubo;

class View : public VulkanView {
public:
  View(const std::shared_ptr<VulkanDevice> &dev) : VulkanView(dev)
  {
    create_sphere();
    create_pipe_layout();
  }

  ~View()
  {
    vkDeviceWaitIdle(*device());

    if (_vert_buf) {
      vkDestroyBuffer(*device(), _vert_buf, nullptr);
      _vert_buf = VK_NULL_HANDLE;
    }

    if (_vert_mem) {
      vkFreeMemory(*device(), _vert_mem, nullptr);
      _vert_mem = VK_NULL_HANDLE;
    }

    if (_index_buf) {
      vkDestroyBuffer(*device(), _index_buf, nullptr);
      _index_buf = VK_NULL_HANDLE;
    }

    if (_index_mem) {
      vkFreeMemory(*device(), _index_mem, nullptr);
      _index_mem = VK_NULL_HANDLE;
    }

    if (_descript_pool) {
      vkDestroyDescriptorPool(*device(), _descript_pool, nullptr);
      _descript_pool = VK_NULL_HANDLE;
    }

    if (_pipe_layout) {
      vkDestroyPipelineLayout(*device(), _pipe_layout, nullptr);
      _pipe_layout = VK_NULL_HANDLE;
    }

    if (_pipeline) {
      vkDestroyPipeline(*device(), _pipeline, nullptr);
      _pipeline = VK_NULL_HANDLE;
    }
  }

  void resize(int w, int h) { update_ubo(); }

  void update_scene()
  {
    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);

    bool overlay = ImGui::Begin("test");
    ImGui::Text("wtf");
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();

    //if (overlay)
    //  update_overlay();
  }

  void build_command_buffer(VkCommandBuffer cmd_buf) override
  {
    if (!_pipeline)
      return;
    {
      vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipeline);
    }

    VkDescriptorSet dessets[2] = {_matrix_set, _material_set};
    vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipe_layout, 0, 1, dessets, 0, nullptr);

    {
      VkDeviceSize offset[2] = {0, _vert_count * sizeof(vec3)};
      VkBuffer bufs[2] = {};
      bufs[0] = _vert_buf;
      bufs[1] = _vert_buf;
      vkCmdBindVertexBuffers(cmd_buf, 0, 2, bufs, offset);
      vkCmdBindIndexBuffer(cmd_buf, _index_buf, 0, VK_INDEX_TYPE_UINT16);
      vkCmdDrawIndexed(cmd_buf, _index_count, 1, 0, 0, 0);
    }
  }

  void create_pipe_layout()
  {
    VkDescriptorSetLayoutBinding layoutBinding[2] = {};
    layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding[0].descriptorCount = 1;
    layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding[0].pImmutableSamplers = nullptr;

    layoutBinding[1].binding = 1;
    layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding[1].descriptorCount = 1;
    layoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding[1].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = 2;
    descriptorLayout.pBindings = layoutBinding;

    VkDescriptorSetLayout matrix_lay, material_lay;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*device(), &descriptorLayout, nullptr, &matrix_lay));

    {
      VkDescriptorSetLayoutBinding material_binding = {};
      material_binding.binding = 2;
      material_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      material_binding.descriptorCount = 1;
      material_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

      descriptorLayout.bindingCount = 1;
      descriptorLayout.pBindings = &material_binding;

      VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*device(), &descriptorLayout, nullptr, &material_lay));
    }

    VkDescriptorSetLayout layouts[2] = {matrix_lay, material_lay};

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 2;
    pPipelineLayoutCreateInfo.pSetLayouts = layouts;

    VkPipelineLayout pipe_layout;
    VK_CHECK_RESULT(vkCreatePipelineLayout(*device(), &pPipelineLayoutCreateInfo, nullptr, &pipe_layout));
    _pipe_layout = pipe_layout;

    //----------------------------------------------------------------------------------------------------

    VkDescriptorPoolSize typeCounts[1];
    typeCounts[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    typeCounts[0].descriptorCount = 10;

    VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.pNext = nullptr;
    descriptorPoolInfo.poolSizeCount = 1;
    descriptorPoolInfo.pPoolSizes = typeCounts;
    descriptorPoolInfo.maxSets = 10;

    VkDescriptorPool desPool;
    VK_CHECK_RESULT(vkCreateDescriptorPool(*device(), &descriptorPoolInfo, nullptr, &desPool));
    _descript_pool = desPool;

    //----------------------------------------------------------------------------------------------------

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = desPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &matrix_lay;

    VK_CHECK_RESULT(vkAllocateDescriptorSets(*device(), &allocInfo, &_matrix_set));

    int sz = sizeof(matrix_ubo);
    VkDescriptorBufferInfo descriptor = {};
    _ubo_buf = device()->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz);
    descriptor.buffer = *_ubo_buf;
    descriptor.offset = 0;
    descriptor.range = sizeof(matrix_ubo);

    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = _matrix_set;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptor;
    writeDescriptorSet.dstBinding = 0;

    vkUpdateDescriptorSets(*device(), 1, &writeDescriptorSet, 0, nullptr);

    vkDestroyDescriptorSetLayout(*device(), matrix_lay, nullptr);
    vkDestroyDescriptorSetLayout(*device(), material_lay, nullptr);
  }

  void create_pipeline()
  {
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = _pipe_layout;
    pipelineCreateInfo.renderPass = render_pass();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;

    VkPipelineRasterizationStateCreateInfo rasterizationState = {};
    rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationState.depthClampEnable = VK_FALSE;
    rasterizationState.rasterizerDiscardEnable = VK_FALSE;
    rasterizationState.depthBiasEnable = VK_FALSE;
    rasterizationState.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
    blendAttachmentState[0].colorWriteMask = 0xf;
    blendAttachmentState[0].blendEnable = VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlendState = {};
    colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendState.attachmentCount = 1;
    colorBlendState.pAttachments = blendAttachmentState;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    std::vector<VkDynamicState> dynamicStateEnables;
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pDynamicStates = dynamicStateEnables.data();
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
    depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilState.depthBoundsTestEnable = VK_FALSE;
    depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
    depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
    depthStencilState.stencilTestEnable = VK_FALSE;
    depthStencilState.front = depthStencilState.back;

    VkPipelineMultisampleStateCreateInfo multisampleState = {};
    multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleState.pSampleMask = nullptr;

    VkVertexInputBindingDescription vertexInputBindings[2] = {};
    vertexInputBindings[0].binding = 0;  // vkCmdBindVertexBuffers
    vertexInputBindings[0].stride = sizeof(vec3);
    vertexInputBindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertexInputBindings[1].binding = 1;  // vkCmdBindVertexBuffers
    vertexInputBindings[1].stride = sizeof(vec3);
    vertexInputBindings[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription vertexInputAttributs[2] = {};
    vertexInputAttributs[0].binding = 0;
    vertexInputAttributs[0].location = 0;
    vertexInputAttributs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[0].offset = 0;
    vertexInputAttributs[1].binding = 1;
    vertexInputAttributs[1].location = 1;
    vertexInputAttributs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vertexInputAttributs[1].offset = 0;

    VkPipelineVertexInputStateCreateInfo vertexInputState = {};
    vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputState.vertexBindingDescriptionCount = 2;
    vertexInputState.pVertexBindingDescriptions = vertexInputBindings;
    vertexInputState.vertexAttributeDescriptionCount = 2;
    vertexInputState.pVertexAttributeDescriptions = vertexInputAttributs;

    VkPipelineShaderStageCreateInfo shaderStages[2] = {};
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = device()->create_shader(SHADER_DIR "/pbr.vert.spv");
    shaderStages[0].pName = "main";
    assert(shaderStages[0].module != VK_NULL_HANDLE);

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = device()->create_shader(SHADER_DIR "/pbr.frag.spv");
    shaderStages[1].pName = "main";
    assert(shaderStages[1].module != VK_NULL_HANDLE);

    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;

    pipelineCreateInfo.pVertexInputState = &vertexInputState;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
    pipelineCreateInfo.pRasterizationState = &rasterizationState;
    pipelineCreateInfo.pColorBlendState = &colorBlendState;
    pipelineCreateInfo.pMultisampleState = &multisampleState;
    pipelineCreateInfo.pViewportState = &viewportState;
    pipelineCreateInfo.pDepthStencilState = &depthStencilState;
    pipelineCreateInfo.pDynamicState = &dynamicState;

    VK_CHECK_RESULT(vkCreateGraphicsPipelines(*device(), device()->get_or_create_pipecache(), 1, &pipelineCreateInfo, nullptr, &_pipeline));

    vkDestroyShaderModule(*device(), shaderStages[0].module, nullptr);
    vkDestroyShaderModule(*device(), shaderStages[1].module, nullptr);

    rebuild_command();
  }

  void wheel(int delta) { update_ubo(); }
  void left_drag(int x, int y, int, int) { update_ubo(); }
  void right_drag(int x, int y, int, int) { update_ubo(); }
  void key_up(int key)
  {
    if (key == SDL_SCANCODE_SPACE)
      update_ubo();
  }

  void update_ubo()
  {
    matrix_ubo.cam = manipulator().eye();
    matrix_ubo.view = manipulator().view_matrix();
    matrix_ubo.model = tg::mat4::identity();
    matrix_ubo.prj = tg::perspective<float>(fov, float(width()) / height(), 0.1, 1000);
    // tg::near_clip(matrix_ubo.prj, tg::vec4(0, 0, -1, 0.5));
    uint8_t *data = 0;
    VK_CHECK_RESULT(vkMapMemory(*device(), _ubo_buf->memory(), 0, sizeof(matrix_ubo), 0, (void **)&data));
    memcpy(data, &matrix_ubo, sizeof(matrix_ubo));
    vkUnmapMemory(*device(), _ubo_buf->memory());
  }

  void create_sphere()
  {
    Sphere sp(vec3(0), 1);
    sp.build();
    auto &verts = sp.get_vertex();
    auto &norms = sp.get_norms();
    // auto &uv = sp.get_uvs();
    auto &index = sp.get_index();
    _vert_count = verts.size();
    _index_count = index.size();

    struct StageBuffer {
      VkBuffer buffer;
      VkDeviceMemory mem;
    };
    StageBuffer vertices, indices;

    uint64_t vert_size = verts.size() * (sizeof(vec3) * 2 + sizeof(vec2));
    VkBufferCreateInfo vertexBufferInfo = {};
    vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vertexBufferInfo.size = vert_size;
    vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VK_CHECK_RESULT(vkCreateBuffer(*device(), &vertexBufferInfo, nullptr, &vertices.buffer));
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(*device(), vertices.buffer, &memReqs);

    VkMemoryAllocateInfo memAlloc = {};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = *device()->memory_type_index(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(*device(), &memAlloc, nullptr, &vertices.mem));

    void *data = 0;
    VK_CHECK_RESULT(vkMapMemory(*device(), vertices.mem, 0, memAlloc.allocationSize, 0, &data));
    uint64_t offset = 0;
    memcpy(data, verts.data(), verts.size() * sizeof(vec3));
    offset += verts.size() * sizeof(vec3);
    memcpy((uint8_t *)data + offset, norms.data(), norms.size() * sizeof(vec3));
    offset += norms.size() * sizeof(vec3);
    // memcpy((uint8_t *)data + offset, uv.data(), uv.size() * sizeof(vec2));
    vkUnmapMemory(*device(), vertices.mem);
    VK_CHECK_RESULT(vkBindBufferMemory(*device(), vertices.buffer, vertices.mem, 0));

    vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VK_CHECK_RESULT(vkCreateBuffer(*device(), &vertexBufferInfo, nullptr, &_vert_buf));
    vkGetBufferMemoryRequirements(*device(), _vert_buf, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = *device()->memory_type_index(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(*device(), &memAlloc, nullptr, &_vert_mem));
    VK_CHECK_RESULT(vkBindBufferMemory(*device(), _vert_buf, _vert_mem, 0));

    uint64_t index_size = index.size() * sizeof(uint16_t);
    VkBufferCreateInfo indexbufferInfo = {};
    indexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexbufferInfo.size = index_size;
    indexbufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    // Copy index data to a buffer visible to the host (staging buffer)
    VK_CHECK_RESULT(vkCreateBuffer(*device(), &indexbufferInfo, nullptr, &indices.buffer));
    vkGetBufferMemoryRequirements(*device(), indices.buffer, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = *device()->memory_type_index(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(*device(), &memAlloc, nullptr, &indices.mem));
    VK_CHECK_RESULT(vkMapMemory(*device(), indices.mem, 0, index_size, 0, &data));
    memcpy(data, index.data(), index_size);
    vkUnmapMemory(*device(), indices.mem);
    VK_CHECK_RESULT(vkBindBufferMemory(*device(), indices.buffer, indices.mem, 0));

    // Create destination buffer with device only visibility
    indexbufferInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    VK_CHECK_RESULT(vkCreateBuffer(*device(), &indexbufferInfo, nullptr, &_index_buf));
    vkGetBufferMemoryRequirements(*device(), _index_buf, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = *device()->memory_type_index(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    VK_CHECK_RESULT(vkAllocateMemory(*device(), &memAlloc, nullptr, &_index_mem));
    VK_CHECK_RESULT(vkBindBufferMemory(*device(), _index_buf, _index_mem, 0));

    {
      VkCommandBuffer cmdBuffer;

      VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
      cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      cmdBufAllocateInfo.commandPool = device()->command_pool();
      cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      cmdBufAllocateInfo.commandBufferCount = 1;

      VK_CHECK_RESULT(vkAllocateCommandBuffers(*device(), &cmdBufAllocateInfo, &cmdBuffer));

      VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
      VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));

      VkBufferCopy copyRegion = {};
      copyRegion.size = vert_size;
      vkCmdCopyBuffer(cmdBuffer, vertices.buffer, _vert_buf, 1, &copyRegion);

      copyRegion.size = index_size;
      vkCmdCopyBuffer(cmdBuffer, indices.buffer, _index_buf, 1, &copyRegion);

      VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &cmdBuffer;

      VkFenceCreateInfo fenceCreateInfo = {};
      fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceCreateInfo.flags = 0;
      VkFence fence;
      VK_CHECK_RESULT(vkCreateFence(*device(), &fenceCreateInfo, nullptr, &fence));

      VK_CHECK_RESULT(vkQueueSubmit(device()->transfer_queue(), 1, &submitInfo, fence));
      VK_CHECK_RESULT(vkWaitForFences(*device(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
      vkDestroyFence(*device(), fence, nullptr);
      vkFreeCommandBuffers(*device(), device()->command_pool(), 1, &cmdBuffer);
    }

    vkDestroyBuffer(*device(), vertices.buffer, nullptr);
    vkDestroyBuffer(*device(), indices.buffer, nullptr);
    vkFreeMemory(*device(), vertices.mem, nullptr);
    vkFreeMemory(*device(), indices.mem, nullptr);
  }

private:
  VkBuffer _vert_buf;
  VkDeviceMemory _vert_mem;
  VkBuffer _index_buf;
  VkDeviceMemory _index_mem;

  VkDescriptorPool _descript_pool = VK_NULL_HANDLE;

  VkDescriptorSet _matrix_set = VK_NULL_HANDLE;
  VkDescriptorSet _material_set = VK_NULL_HANDLE;

  VkPipelineLayout _pipe_layout = VK_NULL_HANDLE;
  VkPipeline _pipeline = VK_NULL_HANDLE;

  std::shared_ptr<VulkanBuffer> _ubo_buf;

  std::shared_ptr<VulkanBuffer> _material_buf;

  uint32_t _vert_count = 0;
  uint32_t _index_count = 0;
};

int main(int argc, char **argv)
{
  SDL_Window *win = 0;
  VkSurfaceKHR surface = 0;
  std::shared_ptr<View> view = 0;
  try {
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      throw std::runtime_error("sdl init error.");
    win = SDL_CreateWindow("demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    if (win == nullptr)
      throw std::runtime_error("could not create sdl window.");

    int w = 0, h = 0;
    SDL_GetWindowSize(win, &w, &h);

    if (!SDL_Vulkan_CreateSurface(win, inst, &surface))
      throw std::runtime_error("could not create vk surface.");

    inst.enable_debug();
    auto dev = inst.create_device();

    view = std::make_shared<View>(dev);
    view->set_surface(surface, w, h);
    view->create_pipeline();
  } catch (std::runtime_error &e) {
    printf("%s", e.what());
    return -1;
  }
  view->frame();
  return 0;
}