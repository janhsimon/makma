#pragma once

#include <vulkan\vulkan.hpp>

struct Vulkan
{
	vk::Instance instance;
	vk::SurfaceKHR surface;
	vk::PhysicalDevice physicalDevice;
	vk::Device device;
	vk::Queue queue;
	vk::CommandPool commandPool;
	vk::SwapchainKHR swapchain;
	std::vector<vk::Image> swapchainImages;
	std::vector<vk::ImageView> swapchainImageViews;
	std::vector<vk::Framebuffer> swapchainFramebuffers;
	std::vector<vk::CommandBuffer> swapchainCommandBuffers;
	vk::PipelineLayout pipelineLayout;
	vk::RenderPass renderPass;
	vk::Pipeline pipeline;
	vk::Buffer vertexBuffer, indexBuffer;
	vk::DeviceMemory vertexBufferMemory, indexBufferMemory;
	vk::Semaphore imageAvailableSemaphore, renderFinishedSemaphore;

	uint32_t queueFamilyIndex = 0;
	vk::SurfaceFormatKHR surfaceFormat;
	vk::PresentModeKHR presentMode;

#ifdef _DEBUG
	VkDebugReportCallbackEXT debugReportCallback;
#endif
};

extern Vulkan vulkan;
