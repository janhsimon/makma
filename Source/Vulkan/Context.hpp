#pragma once

#include "..\Window.hpp"

#include <vulkan\vulkan.hpp>

#include <functional>

class Context
{
private:
	static vk::Instance *createInstance(const Window *window);
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

	static vk::SurfaceKHR *createSurface(const Window *window, const vk::Instance *instance);
	std::function<void(vk::SurfaceKHR*)> surfaceDeleter = [this](vk::SurfaceKHR *surface) { if (instance) instance->destroySurfaceKHR(*surface); };
	std::unique_ptr<vk::SurfaceKHR, decltype(surfaceDeleter)> surface;

	static vk::PhysicalDevice *selectPhysicalDevice(const vk::Instance *instance);
	std::unique_ptr<vk::PhysicalDevice> physicalDevice;

	static vk::Device *createDevice(const vk::SurfaceKHR *surface, const vk::PhysicalDevice *physicalDevice, uint32_t &queueFamilyIndex);
	std::function<void(vk::Device*)> deviceDeleter = [](vk::Device *device) { device->destroy(); };
	std::unique_ptr<vk::Device, decltype(deviceDeleter)> device;

	static vk::CommandPool *createCommandPool(const vk::Device *device, uint32_t queueFamilyIndex);
	std::function<void(vk::CommandPool*)> commandPoolDeleter = [this](vk::CommandPool *commandPool) { if (device) device->destroyCommandPool(*commandPool); };
	std::unique_ptr<vk::CommandPool, decltype(commandPoolDeleter)> commandPool;

	uint32_t queueFamilyIndex;
	vk::Queue queue;

public:
	Context(const Window *window);

	vk::Instance *getInstance() const { return instance.get(); }
	vk::SurfaceKHR *getSurface() const { return surface.get(); }
	vk::PhysicalDevice *getPhysicalDevice() const { return physicalDevice.get(); }
	vk::Device *getDevice() const { return device.get(); }
	vk::CommandPool *getCommandPool() const { return commandPool.get(); }
	uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }
	vk::Queue getQueue() const { return queue; }
};