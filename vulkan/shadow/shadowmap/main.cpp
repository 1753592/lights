#include <exception>
#include <stdexcept>

#include "config.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_vulkan.h"

#include "VulkanDebug.h"
#include "VulkanView.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanBuffer.h"
#include "VulkanImage.h"
#include "VulkanTools.h"
#include "VulkanInitializers.hpp"

#include "BasicPipeline.h"
#include "DepthPipeline.h"

#include "SimpleShape.h"
#include "RenderData.h"
#include "GLTFLoader.h"
#include "MeshInstance.h"

#include "imgui/imgui.h"

#define WM_PAINT 1
#define SHADER_DIR ROOT_DIR##"/vulkan/shadow/shadowmap"

constexpr float fov = 60;

VulkanInstance &inst = VulkanInstance::instance();

PBRBasic pbr;
PointLight light;

class View : public VulkanView {
public:
  View(const std::shared_ptr<VulkanDevice> &dev) : VulkanView(dev, false)
  {
    std::cout << 1;
    create_sphere();

    GLTFLoader loader(dev);
    _tree = loader.load_file(ROOT_DIR "/data/oaktree.gltf");

    _basic_pipeline = std::make_shared<BasicPipeline>(dev);

    //_depth_pipeline = std::make_shared<DepthPipeline>(dev, 2048, 2048);
    _depth_image = _device->create_depth_image(2048, 2048, VK_FORMAT_D32_SFLOAT);

    create_pipe_layout();

    set_uniforms();
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
  }

  void resize(int w, int h) { update_ubo(); }

  void update_scene()
  {
    if (_imgui) {
      ImGui::NewFrame();
      ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);

      bool overlay = ImGui::Begin("test");
      ImGui::Text("wtf");
      ImGui::End();
      ImGui::EndFrame();
      ImGui::Render();
    }

    // if (overlay)
    //   update_overlay();
  }

  void create_command_buffers()
  {
    int count = _swapchain->image_count();
    if (count != _cmd_bufs.size()) {
      _cmd_bufs = _device->create_command_buffers(count);
    }
  }

  void build_command_buffer(VkCommandBuffer cmd_buf) override
  {
    {
      if (_depth_pipeline) {
        vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, *_depth_pipeline);
        vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _depth_pipeline->pipe_layout(), 0, 1, &_depth_matrix_set, 0, nullptr);

        VkDeviceSize offset[2] = {0, _vert_count * sizeof(vec3)};
        VkBuffer bufs[2] = {};
        bufs[0] = _vert_buf;
        bufs[1] = _vert_buf;
        vkCmdBindVertexBuffers(cmd_buf, 0, 1, bufs, offset);
        vkCmdBindIndexBuffer(cmd_buf, _index_buf, 0, VK_INDEX_TYPE_UINT16);
        vkCmdDrawIndexed(cmd_buf, _index_count, 1, 0, 0, 0);
      }
    }

    vkCmdNextSubpass(cmd_buf, VK_SUBPASS_CONTENTS_INLINE);

    {
      VkViewport viewport = {};
      viewport.y = _h;
      viewport.width = _w;
      viewport.height = -_h;
      viewport.minDepth = 0;
      viewport.maxDepth = 1;
      vkCmdSetViewport(cmd_buf, 0, 1, &viewport);
    }

    {
      VkRect2D scissor = {};
      scissor.extent.width = _w;
      scissor.extent.height = _h;
      scissor.offset.x = 0;
      scissor.offset.y = 0;
      vkCmdSetScissor(cmd_buf, 0, 1, &scissor);
    }

    if (_basic_pipeline && _basic_pipeline->valid()) {
      vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, *_basic_pipeline);

      VkDescriptorSet dessets[2] = {_matrix_set, _material_set};
      vkCmdBindDescriptorSets(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, _basic_pipeline->pipe_layout(), 0, 1, dessets, 0, nullptr);

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

    //_tree->build_command_buffer(cmd_buf);
  }

  void create_pipe_layout()
  {
    VkDescriptorSetLayoutBinding layoutBinding[4] = {};
    layoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding[0].descriptorCount = 1;
    layoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding[0].pImmutableSamplers = nullptr;

    layoutBinding[1].binding = 1;
    layoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding[1].descriptorCount = 1;
    layoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding[1].pImmutableSamplers = nullptr;

    layoutBinding[2].binding = 2;
    layoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding[2].descriptorCount = 1;
    layoutBinding[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layoutBinding[2].pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
    descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptorLayout.pNext = nullptr;
    descriptorLayout.bindingCount = 3;
    descriptorLayout.pBindings = layoutBinding;

    VkDescriptorSetLayout matrix_lay, material_lay;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*device(), &descriptorLayout, nullptr, &matrix_lay));
    _basic_pipeline->set_descriptor_layout(matrix_lay);

    VkDescriptorSetLayout layouts[2] = {matrix_lay, material_lay};

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = layouts;

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

    VkDescriptorBufferInfo descriptor = {};
    int sz = sizeof(_matrix);
    _ubo_buf = device()->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz);
    descriptor.buffer = *_ubo_buf;
    descriptor.offset = 0;
    descriptor.range = sizeof(_matrix);

    sz = sizeof(light);
    VkDescriptorBufferInfo ldescriptor = {};
    _light = device()->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz);
    ldescriptor.buffer = *_light;
    ldescriptor.offset = 0;
    ldescriptor.range = sz;

    sz = sizeof(pbr);
    VkDescriptorBufferInfo mdescriptor = {};
    _material = device()->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sz);
    mdescriptor.buffer = *_material;
    mdescriptor.offset = 0;
    mdescriptor.range = sz;


    VkWriteDescriptorSet writeDescriptorSet = {};
    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = _matrix_set;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    writeDescriptorSet.pBufferInfo = &descriptor;
    writeDescriptorSet.dstBinding = 0;
    vkUpdateDescriptorSets(*device(), 1, &writeDescriptorSet, 0, nullptr);

    writeDescriptorSet.pBufferInfo = &mdescriptor;
    writeDescriptorSet.dstBinding = 1;
    vkUpdateDescriptorSets(*device(), 1, &writeDescriptorSet, 0, nullptr);

    writeDescriptorSet.pBufferInfo = &ldescriptor;
    writeDescriptorSet.dstBinding = 2;
    vkUpdateDescriptorSets(*device(), 1, &writeDescriptorSet, 0, nullptr);
  }

  void create_render_pass()
  {
    VkRenderPass render_pass = VK_NULL_HANDLE;

    VkAttachmentDescription attachments[3] = {};
    attachments[0].format = VK_FORMAT_D32_SFLOAT;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    attachments[1].format = swapchain()->color_format();
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[2].format = _depth_format;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthReference = {};
    depthReference.attachment = 0;
    depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference mainColorReference, mainDepthReference;
    mainColorReference.attachment = 1;
    mainColorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    mainDepthReference.attachment = 2;
    mainDepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass[2] = {0};
    subpass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass[0].colorAttachmentCount = 0;
    //subpass[0].pColorAttachments = &mainColorReference;
    subpass[0].pDepthStencilAttachment = &depthReference;

    subpass[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass[1].colorAttachmentCount = 1;
    subpass[1].pColorAttachments = &mainColorReference;
    subpass[1].pDepthStencilAttachment = &mainDepthReference;
    subpass[1].inputAttachmentCount = 0;
    subpass[1].pInputAttachments = nullptr;

    VkSubpassDependency dependencies[3] = {0};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = 0;
    dependencies[2].dstSubpass = 1;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].srcAccessMask = 0;
    dependencies[2].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassCreateInfo = vks::initializers::renderPassCreateInfo();
    renderPassCreateInfo.attachmentCount = 3;
    renderPassCreateInfo.pAttachments = attachments;
    renderPassCreateInfo.subpassCount = 2;
    renderPassCreateInfo.pSubpasses = subpass;
    renderPassCreateInfo.dependencyCount = 3;
    renderPassCreateInfo.pDependencies = dependencies;

    VkRenderPass vkpass = VK_NULL_HANDLE;
    VK_CHECK_RESULT(vkCreateRenderPass(*device(), &renderPassCreateInfo, nullptr, &vkpass));

    set_render_pass(vkpass);

    //_tree->create_pipeline(vkpass);
  }

  void create_frame_buffers()
  {
    VkImageView attachments[3];
    attachments[0] = _depth_image->image_view();
    attachments[2] = _depth->image_view();

    VkFramebufferCreateInfo frameBufferCreateInfo = {};
    frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferCreateInfo.pNext = NULL;
    frameBufferCreateInfo.renderPass = render_pass();
    frameBufferCreateInfo.attachmentCount = 3;
    frameBufferCreateInfo.pAttachments = attachments;
    frameBufferCreateInfo.width = _w;
    frameBufferCreateInfo.height = _h;
    frameBufferCreateInfo.layers = 1;

    std::vector<VkFramebuffer> frameBuffers;
    frameBuffers.resize(_swapchain->image_count());
    for (uint32_t i = 0; i < frameBuffers.size(); i++) {
      attachments[1] = _swapchain->image_view(i);
      VK_CHECK_RESULT(vkCreateFramebuffer(*_device, &frameBufferCreateInfo, nullptr, &frameBuffers[i]));
    }

    set_frame_buffers(frameBuffers);
  }

  void create_pipeline()
  {
    if (_depth_pipeline) {
      _depth_pipeline->realize(render_pass());

      auto des_layout = _depth_pipeline->descriptor_layout();
      VkDescriptorSetAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      allocInfo.descriptorPool = _descript_pool;
      allocInfo.descriptorSetCount = 1;
      allocInfo.pSetLayouts = &des_layout;

      VK_CHECK_RESULT(vkAllocateDescriptorSets(*device(), &allocInfo, &_depth_matrix_set));

      int sz = sizeof(_depth_matrix);
      VkDescriptorBufferInfo descriptor = {};
      descriptor.buffer = *_depth_matrix_buf;
      descriptor.offset = 0;
      descriptor.range = sz;

      VkWriteDescriptorSet writeDescriptorSet = {};
      writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      writeDescriptorSet.dstSet = _depth_matrix_set;
      writeDescriptorSet.descriptorCount = 1;
      writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      writeDescriptorSet.pBufferInfo = &descriptor;
      writeDescriptorSet.dstBinding = 0;
      vkUpdateDescriptorSets(*device(), 1, &writeDescriptorSet, 0, nullptr);
    }

    _basic_pipeline->realize(render_pass(), 1);

    build_command_buffers();
  }

  void build_command_buffers()
  {
    auto &framebuffers = frame_buffers();
    auto &renderPass = render_pass();
    vkDeviceWaitIdle(*_device);

    assert(framebuffers.size() == _cmd_bufs.size());

    VkCommandBufferBeginInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buf_info.pNext = nullptr;

    VkClearValue clearValues[3];
    clearValues[0].depthStencil = {1.f, 0};
    clearValues[1].color = {{0.0, 0.0, 0.2, 1.0}};
    clearValues[2].depthStencil = {1.f, 0};

    VkRenderPassBeginInfo renderPassBeginInfo = {};
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.renderArea.offset.x = 0;
    renderPassBeginInfo.renderArea.offset.y = 0;
    renderPassBeginInfo.renderArea.extent.width = _w;
    renderPassBeginInfo.renderArea.extent.height = _h;
    renderPassBeginInfo.clearValueCount = 3;
    renderPassBeginInfo.pClearValues = clearValues;

    for (int i = 0; i < _cmd_bufs.size(); i++) {
      renderPassBeginInfo.framebuffer = framebuffers[i];
      auto &cmd_buf = _cmd_bufs[i];
      VK_CHECK_RESULT(vkBeginCommandBuffer(cmd_buf, &buf_info));

      vkCmdBeginRenderPass(cmd_buf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

      build_command_buffer(cmd_buf);

      vkCmdEndRenderPass(cmd_buf);

      VK_CHECK_RESULT(vkEndCommandBuffer(cmd_buf));
    }
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
    _matrix.eye = manipulator().eye();
    _matrix.view = manipulator().view_matrix();
    _matrix.model = tg::mat4::identity();
    _matrix.prj = tg::perspective<float>(fov, float(width()) / height(), 0.1, 1000);
    // tg::near_clip(_matrix.prj, tg::vec4(0, 0, -1, 0.5));
    uint8_t *data = 0;
    VK_CHECK_RESULT(vkMapMemory(*device(), _ubo_buf->memory(), 0, sizeof(_matrix), 0, (void **)&data));
    memcpy(data, &_matrix, sizeof(_matrix));
    vkUnmapMemory(*device(), _ubo_buf->memory());

    auto mvp = _matrix;
    mvp.model = tg::translate(vec3(0, 0, 1));
    _tree->set_mvp(mvp);
  }

  void set_uniforms()
  {
    vec3 light_pos = vec3(5, 5, 5);

    light.light_pos = light_pos;
    light.light_color = vec3(500);

    uint8_t *data = 0;
    VK_CHECK_RESULT(vkMapMemory(*device(), _light->memory(), 0, sizeof(light), 0, (void **)&data));
    memcpy(data, &light, sizeof(light));

    pbr.albedo = vec3(0.8);
    pbr.ao = 1;
    pbr.metallic = 0.2;
    pbr.roughness = 0.7;
    VK_CHECK_RESULT(vkMapMemory(*device(), _material->memory(), 0, sizeof(pbr), 0, (void **)&data));
    memcpy(data, &pbr, sizeof(pbr));

    //_depth_matrix_buf =
    //    device()->create_buffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(MVP));
    //_depth_matrix.view = tg::lookat(light_pos);
    //_depth_matrix.prj = tg::perspective<float>(60, 1, 0.5, 1000);
    //VK_CHECK_RESULT(vkMapMemory(*device(), _depth_matrix_buf->memory(), 0, sizeof(material_ubo), 0, (void **)&data));
    //memcpy(data, &_depth_matrix, sizeof(MVP));
  }

  void create_sphere()
  {
    Box box(vec3(0), vec3(20, 20, 2));
    box.build();
    auto &verts = box.get_vertex();
    auto &norms = box.get_norms();
    // auto &uv = sp.get_uvs();
    auto &index = box.get_index();
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

  std::shared_ptr<BasicPipeline> _basic_pipeline;
  std::shared_ptr<DepthPipeline> _depth_pipeline;

  VkDescriptorSet _matrix_set = VK_NULL_HANDLE;
  VkDescriptorSet _material_set = VK_NULL_HANDLE;

  VkDescriptorSet _depth_matrix_set = VK_NULL_HANDLE;
  std::shared_ptr<VulkanBuffer> _depth_matrix_buf;

  std::shared_ptr<VulkanImage> _depth_image;

  std::shared_ptr<VulkanBuffer> _ubo_buf, _light, _material;

  MVP _matrix, _depth_matrix;

  uint32_t _vert_count = 0;
  uint32_t _index_count = 0;

  std::shared_ptr<MeshInstance> _tree;
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
    view->create_render_pass();
    view->create_pipeline();
  } catch (std::runtime_error &e) {
    printf("%s", e.what());
    return -1;
  }
  view->frame();
  return 0;
}