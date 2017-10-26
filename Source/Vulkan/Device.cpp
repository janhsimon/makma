#include "Device.hpp"
#include "Vulkan.hpp"
#include "..\Window.hpp"

#include <vector>

bool Device::selectPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	if (vulkan.instance.enumeratePhysicalDevices(&physicalDeviceCount, nullptr) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to enumerate physical devices.");
		return false;
	}

	if (physicalDeviceCount <= 0)
	{
		Window::showMessageBox("Error", "No physical devices found.");
		return false;
	}

	std::vector<vk::PhysicalDevice> physicalDevices(physicalDeviceCount);
	if (vulkan.instance.enumeratePhysicalDevices(&physicalDeviceCount, physicalDevices.data()) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to enumerate physical devices.");
		return false;
	}

	if (physicalDeviceCount == 1)
	{
		vulkan.physicalDevice = physicalDevices[0];
		return true;
	}

	std::vector<std::string> physicalDeviceNames(physicalDevices.size());
	for (uint32_t i = 0; i < physicalDeviceNames.size(); ++i)
	{
		physicalDeviceNames[i] = physicalDevices[i].getProperties().deviceName;
	}

	int buttonID;
	Window::showChoiceBox("Multiple physical devices", "Select your desired physical device:", physicalDeviceNames, buttonID);
	vulkan.physicalDevice = physicalDevices[buttonID];
	return true;
}

bool Device::selectQueueFamilyIndex()
{
	uint32_t queueFamilyPropertyCount = 0;
	vulkan.physicalDevice.getQueueFamilyProperties(&queueFamilyPropertyCount, nullptr);

	if (queueFamilyPropertyCount <= 0)
	{
		Window::showMessageBox("Error", "Failed to enumerate queue family properties for physical device.");
		return false;
	}

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
	vulkan.physicalDevice.getQueueFamilyProperties(&queueFamilyPropertyCount, queueFamilyProperties.data());

	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		if (queueFamilyProperties[i].queueCount > 0 && queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			VkBool32 presentSupport;
			if (vulkan.physicalDevice.getSurfaceSupportKHR(vulkan.queueFamilyIndex, vulkan.surface, &presentSupport) == vk::Result::eSuccess)
			{
				if (presentSupport)
				{
					vulkan.queueFamilyIndex = i;
					return true;
				}
			}
		}
	}

	Window::showMessageBox("Error", "Failed to find suitable queue family for physical device.");
	return false;
}

bool Device::createDevice()
{
	std::vector<float> queuePriorities = { 1.0f };
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(), vulkan.queueFamilyIndex, static_cast<uint32_t>(queuePriorities.size()), queuePriorities.data());

	std::vector<const char *> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(), 1, &deviceQueueCreateInfo, 0, nullptr, static_cast<uint32_t>(deviceExtensions.size()), deviceExtensions.data());

	if (vulkan.physicalDevice.createDevice(&deviceCreateInfo, nullptr, &vulkan.device) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create device.");
		return false;
	}

	vulkan.device.getQueue(vulkan.queueFamilyIndex, 0, &vulkan.queue);

	return true;
}

Device::~Device()
{
	if (vulkan.device)
	{
		vulkan.device.destroy();
	}
}

bool Device::create()
{
	if (!selectPhysicalDevice()) return false;
	if (!selectQueueFamilyIndex()) return false;
	if (!createDevice()) return false;

	return true;
}