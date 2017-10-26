#include "Surface.hpp"
#include "Vulkan.hpp"

#include <SDL_vulkan.h>

Surface::~Surface()
{
	if (vulkan.instance && vulkan.surface)
	{
		vulkan.instance.destroySurfaceKHR(vulkan.surface);
	}
}

bool Surface::create(const Window *window)
{
	auto temp = VkSurfaceKHR(vulkan.surface);
	if (!SDL_Vulkan_CreateSurface(window->getWindow(), vulkan.instance, &temp))
	{
		Window::showMessageBox("Error", "Failed to create Vulkan surface.");
		return false;
	}

	vulkan.surface = temp;

	return true;
}