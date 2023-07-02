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
#include "GLTFLoader.h"
#include "config.h"

#define SHADER_DIR ROOT_DIR##"/vulkan/tbdr"

#define WM_PAINT 1

constexpr float fov = 60;

VulkanInstance &inst = VulkanInstance::instance();

struct {
  tg::mat4 prj;
  tg::mat4 view;
  tg::mat4 model;
  tg::vec4 cam;
} matrix_ubo;

struct {
  struct alignas(16) aligned_vec3 : vec3 {};
  aligned_vec3 light_pos[4] = {vec3(10, -10, 10), vec3(-10, -10, 10), vec3(-10, -10, -10), vec3(10, -10, -10)};
  aligned_vec3 light_color[4] = {vec3(300), vec3(300), vec3(300), vec3(300)};
} lights_ubo;

struct alignas(16) {
  struct alignas(16) aligned_vec3 : vec3 {};

  float metallic;
  float roughness;
  float ao;
  aligned_vec3 albedo;
} material_ubo;

class Test : public VulkanView {
public:
  Test(const std::shared_ptr<VulkanDevice> &dev) : VulkanView(dev) { 
    create_pipe_layout(); 

    GLTFLoader loader(_device);
    loader.load_file(ROOT_DIR "/data/deer.gltf");
  }

  ~Test() {}

  void set_window(SDL_Window *win)
  {
    int w = 0, h = 0;
    SDL_GetWindowSize(win, &w, &h);

    VkSurfaceKHR surface;
    if (!SDL_Vulkan_CreateSurface(win, inst, &surface))
      throw std::runtime_error("could not create vk surface.");

    set_surface(surface, w, h);

    create_pipeline();
  }

  void resize(int, int) {}
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

  void create_pipe_layout()
  {
    VkDescriptorSetLayoutBinding layoutBinding[3] = {};
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

    VkDescriptorSetLayout descriptor_layout;
    VK_CHECK_RESULT(vkCreateDescriptorSetLayout(*device(), &descriptorLayout, nullptr, &descriptor_layout));


    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &descriptor_layout;

    VkPipelineLayout pipe_layout;
    VK_CHECK_RESULT(vkCreatePipelineLayout(*device(), &pPipelineLayoutCreateInfo, nullptr, &pipe_layout));
    _pipe_layout = pipe_layout;
  }

  void create_pipeline() {
    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.layout = _pipe_layout;
    pipelineCreateInfo.renderPass = render_pass();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
    inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

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

  }

private:
  std::shared_ptr<VulkanBuffer> _ubo_buf;

  VkPipelineLayout _pipe_layout = VK_NULL_HANDLE;

  VkPipeline _pipeline = VK_NULL_HANDLE;
};

int main(int argc, char **argv)
{
  SDL_Window *win = 0;
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

  } catch (std::runtime_error &e) {
    printf("%s", e.what());
    return -1;
  }
  test->frame();
  return 0;
}