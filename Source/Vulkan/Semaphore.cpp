#include "Semaphore.hpp"
#include "Vulkan.hpp"
#include "..\Window.hpp"

Semaphore::~Semaphore()
{
	if (vulkan.device)
	{
		vulkan.device.destroySemaphore(vulkan.imageAvailableSemaphore);
		vulkan.device.destroySemaphore(vulkan.renderFinishedSemaphore);
	}
}

bool Semaphore::create()
{
	vk::SemaphoreCreateInfo semaphoreCreateInfo;
	if (vulkan.device.createSemaphore(&semaphoreCreateInfo, nullptr, &vulkan.imageAvailableSemaphore) != vk::Result::eSuccess ||
		vulkan.device.createSemaphore(&semaphoreCreateInfo, nullptr, &vulkan.renderFinishedSemaphore) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create semaphore.");
		return false;
	}

	return true;
}