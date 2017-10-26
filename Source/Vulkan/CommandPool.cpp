#include "CommandPool.hpp"
#include "Vulkan.hpp"
#include "..\Window.hpp"

CommandPool::~CommandPool()
{
	if (vulkan.device && vulkan.commandPool)
	{
		vulkan.device.destroyCommandPool(vulkan.commandPool);
	}
}

bool CommandPool::create()
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo(vk::CommandPoolCreateFlags(), vulkan.queueFamilyIndex);
	if (vulkan.device.createCommandPool(&commandPoolCreateInfo, nullptr, &vulkan.commandPool) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create command pool.");
		return false;
	}

	return true;
}