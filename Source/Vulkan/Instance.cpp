#include "Instance.hpp"
#include "Vulkan.hpp"

#include <SDL_vulkan.h>

#include <vector>

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFunction(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	Window::showMessageBox("Debug Report", pMessage);
	return VK_FALSE;
}
#endif

Instance::~Instance()
{
	if (vulkan.instance)
	{
#ifdef _DEBUG
		if (vulkan.debugReportCallback)
		{
			// NOTE: instance.destroyDebugReportCallbackEXT() would be much nicer but is not currently implemented by the API (only declared)
			PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vulkan.instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));
			vkDestroyDebugReportCallbackEXT(vulkan.instance, vulkan.debugReportCallback, nullptr);
		}
#endif

		vulkan.instance.destroy();
	}
}

bool Instance::create(const Window *window)
{
	std::vector<const char *> layers;

#ifdef _DEBUG
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

	unsigned int extensionCount = 0;
	if (!SDL_Vulkan_GetInstanceExtensions(window->getWindow(), &extensionCount, nullptr))
	{
		Window::showMessageBox("Error", "Failed to get instance extensions: " + std::string(SDL_GetError()));
		return false;
	}

	std::vector<const char *> extensions(extensionCount);
	if (!SDL_Vulkan_GetInstanceExtensions(window->getWindow(), &extensionCount, extensions.data()))
	{
		Window::showMessageBox("Error", "Failed to get instance extensions: " + std::string(SDL_GetError()));
		return false;
	}

#ifdef _DEBUG
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

	vk::ApplicationInfo applicationInfo("Makma", 1, "Makma", 1, VK_API_VERSION_1_0);
	vk::InstanceCreateInfo instanceCreateInfo(vk::InstanceCreateFlags(), &applicationInfo, static_cast<uint32_t>(layers.size()), layers.data(), static_cast<uint32_t>(extensions.size()), extensions.data());

	if (vk::createInstance(&instanceCreateInfo, nullptr, &vulkan.instance) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create Vulkan instance.");
		return false;
	}

#ifdef _DEBUG
	// NOTE: instance.createDebugReportCallbackEXT() would be much nicer but is not currently implemented by the API (only declared)
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vulkan.instance.getProcAddr("vkCreateDebugReportCallbackEXT"));

	vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo(vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::ePerformanceWarning, debugReportCallbackFunction);
	auto temp = VkDebugReportCallbackCreateInfoEXT(debugReportCallbackCreateInfo);

	if (vkCreateDebugReportCallbackEXT(vulkan.instance, &temp, nullptr, &vulkan.debugReportCallback) != VK_SUCCESS)
	{
		Window::showMessageBox("Error", "Failed to create debug report callback.");
		return false;
	}
#endif

	return true;
}