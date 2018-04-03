#pragma once

#include "../Window.hpp"

#include <vulkan/vulkan.hpp>

#include <functional>

//#define MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL		0 // TODO: currently broken
//#define MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC	1
//#define MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_DYNAMIC			2 // TODO: not implemented yet

#define MK_OPTIMIZATION_COMMAND_POOL_REPEAT_TRANSIENT	// TODO: make into a setting under Memory Management
#define MK_OPTIMIZATION_VERTEX_INDEX_BUFFER_STAGING		// TODO: make into a setting under Memory Management
//#define MK_OPTIMIZATION_RENDER_PARALLEL
//#define MK_OPTIMIZATION_UNIFORM_BUFFER_MODE				MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
#define MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION			4096
#define MK_OPTIMIZATION_SHADOW_MAP_MAX_CASCADE_COUNT	16

class Context
{
private:
	static vk::Instance *createInstance(const std::shared_ptr<Window> window);
	std::function<void(vk::Instance*)> instanceDeleter = [](vk::Instance *instance) { instance->destroy(); };
	std::unique_ptr<vk::Instance, decltype(instanceDeleter)> instance;
	
#ifdef _DEBUG
	static VkDebugReportCallbackEXT *createDebugReportCallback(const vk::Instance *instance);
	std::function<void(VkDebugReportCallbackEXT*)> debugReportCallbackDeleter = [this](VkDebugReportCallbackEXT *debugReportCallback)
	{
		// NOTE: instance.destroyDebugReportCallbackEXT() would be much nicer but is not currently implemented by the API (only declared)
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(instance->getProcAddr("vkDestroyDebugReportCallbackEXT"));
		VkInstance *temp = (VkInstance*)instance.get();
		vkDestroyDebugReportCallbackEXT(*temp, *debugReportCallback, nullptr);
	};
	std::unique_ptr<VkDebugReportCallbackEXT, decltype(debugReportCallbackDeleter)> debugReportCallback;
#endif

	static vk::SurfaceKHR *createSurface(const std::shared_ptr<Window> window, const vk::Instance *instance);
	std::function<void(vk::SurfaceKHR*)> surfaceDeleter = [this](vk::SurfaceKHR *surface) { if (instance) instance->destroySurfaceKHR(*surface); };
	std::unique_ptr<vk::SurfaceKHR, decltype(surfaceDeleter)> surface;

	static vk::PhysicalDevice *selectPhysicalDevice(const vk::Instance *instance);
	std::unique_ptr<vk::PhysicalDevice> physicalDevice;

	static vk::Device *createDevice(const vk::SurfaceKHR *surface, const vk::PhysicalDevice *physicalDevice, uint32_t &queueFamilyIndex);
	std::function<void(vk::Device*)> deviceDeleter = [](vk::Device *device) { device->destroy(); };
	std::unique_ptr<vk::Device, decltype(deviceDeleter)> device;

	static vk::CommandPool *createCommandPoolOnce(const vk::Device *device, uint32_t queueFamilyIndex);
	static vk::CommandPool *createCommandPoolRepeat(const vk::Device *device, uint32_t queueFamilyIndex);
	std::function<void(vk::CommandPool*)> commandPoolDeleter = [this](vk::CommandPool *commandPool) { if (device) device->destroyCommandPool(*commandPool); };
	std::unique_ptr<vk::CommandPool, decltype(commandPoolDeleter)> commandPoolOnce, commandPoolRepeat;

	uint32_t queueFamilyIndex;
	vk::Queue queue;

	uint32_t uniformBufferDataAlignment;
	uint32_t uniformBufferDataAlignmentLarge;

public:
	Context(const std::shared_ptr<Window> window);

	void calculateUniformBufferDataAlignment();

	vk::Instance *getInstance() const { return instance.get(); }
	vk::SurfaceKHR *getSurface() const { return surface.get(); }
	vk::PhysicalDevice *getPhysicalDevice() const { return physicalDevice.get(); }
	vk::Device *getDevice() const { return device.get(); }
	vk::CommandPool *getCommandPoolOnce() const { return commandPoolOnce.get(); }
	vk::CommandPool *getCommandPoolRepeat() const { return commandPoolRepeat.get(); }
	uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }
	vk::Queue getQueue() const { return queue; }
	uint32_t getUniformBufferDataAlignment() const { return uniformBufferDataAlignment; }
	uint32_t getUniformBufferDataAlignmentLarge() const { return uniformBufferDataAlignmentLarge; }
};