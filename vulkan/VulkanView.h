#pragma once

#define VK_USE_PLATFORM_WIN32_KHR

#include <algorithm>
#include <vector>
#include <array>
#include <optional>
#include <set>

#include "vulkan/vulkan.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_system.h"
#include "SDL2/SDL_vulkan.h"

#include "osg/Vec2"
#include "osg/Vec3"
#include "osg/vec4" 
#include "osg/Matrixf"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct Vertex {
	osg::Vec2 pos;
	osg::Vec3 color;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindingDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}
};

struct UniformBufferObject {
	alignas(16) osg::Matrixf model;
	alignas(16) osg::Matrixf view;
	alignas(16) osg::Matrixf proj;
};

class VulkanView {
public:
	VulkanView();
	~VulkanView();

	void    run();
	void	update();
private:
	SDL_Window* _window;
	VkInstance          _instance;
	VkSurfaceKHR        _surface;

	VkPhysicalDevice    _physicalDevice = VK_NULL_HANDLE;
	VkDevice            _device;

	VkQueue             _graphicalQueue;
	VkQueue             _presentQueue;

	VkSwapchainKHR				_swapChain;
	std::vector<VkImage>		_swapChainImages;
	VkFormat					_swapChainImageFormat;
	VkExtent2D					_swapChainExtent;
	std::vector<VkImageView>	_swapChainImageViews;
	std::vector<VkFramebuffer>	_swapChainFramebuffers;

	VkRenderPass				_renderPass;
	VkDescriptorSetLayout		_descriptorSetLayout;
	VkPipelineLayout			_pipelineLayout;
	VkPipeline					_graphicsPipeline;

	VkCommandPool				_commandPool;

	VkBuffer _vertexBuffer;
	VkDeviceMemory _vertexBufferMemory;
	VkBuffer _indexBuffer;
	VkDeviceMemory _indexBufferMemory;

	std::vector<VkBuffer> _uniformBuffers;
	std::vector<VkDeviceMemory> _uniformBuffersMemory;

	VkDescriptorPool				_descriptorPool;
	std::vector<VkDescriptorSet>	_descriptorSets;

	std::vector<VkCommandBuffer>	_commandBuffers;

	std::vector<VkSemaphore>		_imageAvailableSemaphores;
	std::vector<VkSemaphore>		_renderFinishedSemaphores;
	std::vector<VkFence>			_inFlightFences;
	std::vector<VkFence>			_imagesInFlight;

	size_t		_currentFrame = 0;
	bool		_framebufferResized = false;
	bool		_runing = true;

	VkDebugUtilsMessengerEXT    debugMessenger;

protected:
	void initWindow();
	void initVulkan();
	void mainLoop();
	void cleanupSwapChain();
	void cleanup();
	void recreateSwapChain();

	void createInstance();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void setupDebugMessenger();

	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();

	void createVertexBuffer();
	void createIndexBuffer();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	void createCommandBuffers();

	void createSyncObjects();


	void updateUniformBuffer(uint32_t currentImage);
	void drawFrame();

	VkShaderModule createShaderModule(const std::vector<char>& code);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	QueueFamilyIndices			findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*>	getRequiredExtensions();
	static std::vector<char>	readFile(const std::string& filename);
};
