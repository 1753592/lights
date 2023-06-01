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

struct UniformBufferObject {
	osg::Vec3f	camPos;
	alignas(16) osg::Matrixf model;
	alignas(16) osg::Matrixf view;
	alignas(16) osg::Matrixf proj;
};

struct UniformLights {
	alignas(16) osg::Vec3f light1;
	alignas(16) osg::Vec3f light2;
	alignas(16) osg::Vec3f light3;
	alignas(16) osg::Vec3f light4;
	alignas(16) osg::Vec3f lightColor1;
	alignas(16) osg::Vec3f lightColor2;
	alignas(16) osg::Vec3f lightColor3;
	alignas(16) osg::Vec3f lightColor4;
};

struct UniformMaterial {
	float metallic;
	float roughness;
	float ao;
	alignas(16) osg::Vec3 albedo;
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
	VkDescriptorSetLayout		_materialSetLayout;
	VkPipelineLayout			_pipelineLayout;
	VkPipeline					_graphicsPipeline;

	VkCommandPool				_commandPool;

	std::vector<VkBuffer> _uniformBuffers;
	std::vector<VkDeviceMemory> _uniformBuffersMemory;

	std::vector<VkBuffer> _material, _lights;
	std::vector<VkDeviceMemory> _materialMemory, _lightsMemory;

	VkDescriptorPool				_descriptorPool;
	std::vector<VkDescriptorSet>	_descriptorSets;
	std::vector<std::vector<VkDescriptorSet>> _materialSets;

	std::vector<VkCommandBuffer>	_commandBuffers;

	std::vector<VkSemaphore>		_imageAvailableSemaphores;
	std::vector<VkSemaphore>		_renderFinishedSemaphores;
	std::vector<VkFence>			_inFlightFences;
	std::vector<VkFence>			_imagesInFlight;

	VkImage			_depthImage;
	VkDeviceMemory	_depthImageMemory;
	VkImageView		_depthImageView;

	size_t		_minoffset = 0x100;

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
	void createSyncObjects();
	void createDepthRes();

	auto createVertexBuffer(void*, unsigned len);
	auto createIndexBuffer(uint16_t *, unsigned len);
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();

	void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkFormat findDepthFormat();
	bool hasStencilComponent(VkFormat format);
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	virtual void updateUniformBuffer(uint32_t currentImage);
	virtual void drawFrame();

	VkShaderModule createShaderModule(const std::vector<char>& code);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport();

	QueueFamilyIndices			findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*>	getRequiredExtensions();
	static std::vector<char>	readFile(const std::string& filename);

private:
	osg::Vec3 _pos, _cam, _up;
public:
	virtual void createDrawables();
};
