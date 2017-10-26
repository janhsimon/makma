#include "IndexBuffer.hpp"
#include "Swapchain.hpp"
#include "VertexBuffer.hpp"
#include "Vulkan.hpp"

#include <array>

bool Swapchain::checkSurfaceCapabilities()
{
	vk::SurfaceCapabilitiesKHR surfaceCapabilities;
	if (vulkan.physicalDevice.getSurfaceCapabilitiesKHR(vulkan.surface, &surfaceCapabilities) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get surface capabilities for physical device.");
		return false;
	}

	if ((surfaceCapabilities.minImageCount > 2 || surfaceCapabilities.maxImageCount < 2) && surfaceCapabilities.maxImageCount != 0)
	// maxImageCount == 0 means there are no restrictions
	{
		Window::showMessageBox("Error", "The physical device does not meet the surface capability requirements.");
		return false;
	}

	return true;
}

bool Swapchain::selectSurfaceFormat()
{
	uint32_t formatsCount = 0;
	if (vulkan.physicalDevice.getSurfaceFormatsKHR(vulkan.surface, &formatsCount, nullptr) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get supported surface formats for physical device.");
		return false;
	}

	if (formatsCount <= 0)
	{
		Window::showMessageBox("Error", "No supported surface formats found for physical device.");
		return false;
	}

	std::vector<vk::SurfaceFormatKHR> surfaceFormats(formatsCount);
	if (vulkan.physicalDevice.getSurfaceFormatsKHR(vulkan.surface, &formatsCount, surfaceFormats.data()) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get supported surface formats for physical device.");
		return false;
	}

	if (surfaceFormats.size() == 1)
	{
		if (surfaceFormats[0].format == vk::Format::eUndefined)
		// undefined as the only result means there are no preferences
		{
			vulkan.surfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
			vulkan.surfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
			return true;
		}
		
		vulkan.surfaceFormat.format = surfaceFormats[0].format;
		vulkan.surfaceFormat.colorSpace = surfaceFormats[0].colorSpace;
		return true;
	}

	std::vector<std::string> surfaceFormatNames(surfaceFormats.size());
	for (uint32_t i = 0; i < surfaceFormatNames.size(); ++i)
	{
		surfaceFormatNames[i] = vk::to_string(surfaceFormats[i].format) + "/" + vk::to_string(surfaceFormats[i].colorSpace);
	}

	int buttonID;
	Window::showChoiceBox("Multiple surface formats", "Select your desired surface format:", surfaceFormatNames, buttonID);
	vulkan.surfaceFormat.format = surfaceFormats[buttonID].format;
	vulkan.surfaceFormat.colorSpace = surfaceFormats[buttonID].colorSpace;
	return true;
}

bool Swapchain::selectSurfacePresentMode()
{
	uint32_t presentModesCount = 0;
	if (vulkan.physicalDevice.getSurfacePresentModesKHR(vulkan.surface, &presentModesCount, nullptr) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get supported surface present modes for physical device.");
		return false;
	}

	if (presentModesCount <= 0)
	{
		Window::showMessageBox("Error", "No supported surface present modes found for physical device.");
		return false;
	}

	std::vector<vk::PresentModeKHR> presentModes(presentModesCount);
	if (vulkan.physicalDevice.getSurfacePresentModesKHR(vulkan.surface, &presentModesCount, presentModes.data()) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get supported surface present modes for physical device.");
		return false;
	}

	if (presentModesCount == 1)
	{
		vulkan.presentMode = presentModes[0];
		return true;
	}

	std::vector<std::string> presentModeNames(presentModes.size());
	for (uint32_t i = 0; i < presentModeNames.size(); ++i)
	{
		presentModeNames[i] = vk::to_string(presentModes[i]);
	}

	int buttonID;
	Window::showChoiceBox("Multiple present modes", "Select your desired present mode:", presentModeNames, buttonID);
	vulkan.presentMode = presentModes[buttonID];
	return true;
}

bool Swapchain::getImages()
{
	uint32_t imageCount = 0;
	if (vulkan.device.getSwapchainImagesKHR(vulkan.swapchain, &imageCount, nullptr) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get swapchain images.");
		return false;
	}

	if (imageCount <= 0)
	{
		Window::showMessageBox("Error", "No swapchain images in swapchain.");
		return false;
	}

	vulkan.swapchainImages.resize(imageCount);
	if (vulkan.device.getSwapchainImagesKHR(vulkan.swapchain, &imageCount, vulkan.swapchainImages.data()) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to get swapchain images.");
		return false;
	}

	return true;
}

bool Swapchain::createImageViews()
{
	vulkan.swapchainImageViews.resize(vulkan.swapchainImages.size());
	for (size_t i = 0; i < vulkan.swapchainImageViews.size(); ++i)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo(vk::ImageViewCreateFlags(), vulkan.swapchainImages[i], vk::ImageViewType::e2D, vulkan.surfaceFormat.format, vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		if (vulkan.device.createImageView(&imageViewCreateInfo, nullptr, &vulkan.swapchainImageViews[i]) != vk::Result::eSuccess)
		{
			Window::showMessageBox("Error", "Failed to create image view for swapchain image.");
			return false;
		}
	}

	return true;
}

bool Swapchain::createFramebuffers(const Window *window)
{
	vulkan.swapchainFramebuffers.resize(vulkan.swapchainImageViews.size());
	for (size_t i = 0; i < vulkan.swapchainFramebuffers.size(); ++i)
	{
		vk::ImageView attachments[] = { vulkan.swapchainImageViews[i] };
		vk::FramebufferCreateInfo framebufferCreateInfo(vk::FramebufferCreateFlags(), vulkan.renderPass, 1, attachments, window->getWidth(), window->getHeight(), 1);
		if (vulkan.device.createFramebuffer(&framebufferCreateInfo, nullptr, &vulkan.swapchainFramebuffers[i]) != vk::Result::eSuccess)
		{
			Window::showMessageBox("Error", "Failed to create framebuffer.");
			return false;
		}
	}

	return true;
}

bool Swapchain::createCommandBuffers(const Window *window)
{
	vulkan.swapchainCommandBuffers.resize(vulkan.swapchainImageViews.size());

	vk::CommandBufferAllocateInfo commandBufferAllocateInfo(vulkan.commandPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(vulkan.swapchainCommandBuffers.size()));
	if (vulkan.device.allocateCommandBuffers(&commandBufferAllocateInfo, vulkan.swapchainCommandBuffers.data()) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to allocate command buffers.");
		return false;
	}

	for (size_t i = 0; i < vulkan.swapchainCommandBuffers.size(); ++i)
	{
		vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		if (vulkan.swapchainCommandBuffers[i].begin(commandBufferBeginInfo) != vk::Result::eSuccess)
		{
			Window::showMessageBox("Error", "Failed to begin recording command buffer.");
			return false;
		}

		std::array<float, 4> clearColor = { 1.0f, 0.8f, 0.4f, 1.0f };
		
		vk::ClearValue clearValue;
		clearValue.color = clearColor;

		vk::RenderPassBeginInfo renderPassBeginInfo(vulkan.renderPass, vulkan.swapchainFramebuffers[i], vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight())), 1, &clearValue);
		vulkan.swapchainCommandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
	
		vulkan.swapchainCommandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, vulkan.pipeline);

		vk::DeviceSize offsets[] = { 0 };
		vulkan.swapchainCommandBuffers[i].bindVertexBuffers(0, 1, &vulkan.vertexBuffer, offsets);
		vulkan.swapchainCommandBuffers[i].bindIndexBuffer(vulkan.indexBuffer, 0, vk::IndexType::eUint32);

		vulkan.swapchainCommandBuffers[i].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		vulkan.swapchainCommandBuffers[i].endRenderPass();

		if (vulkan.swapchainCommandBuffers[i].end() != vk::Result::eSuccess)
		{
			Window::showMessageBox("Error", "Failed to end recording command buffer.");
			return false;
		}
	}

	return true;
}

Swapchain::~Swapchain()
{
	if (vulkan.device)
	{
		for (auto &framebuffer : vulkan.swapchainFramebuffers)
		{
			vulkan.device.destroyFramebuffer(framebuffer);
		}

		for (auto &imageView : vulkan.swapchainImageViews)
		{
			vulkan.device.destroyImageView(imageView);
		}

		if (vulkan.swapchain)
		{
			vulkan.device.destroySwapchainKHR(vulkan.swapchain);
		}
	}
}

bool Swapchain::create(const Window *window)
{
	if (!checkSurfaceCapabilities()) return false;
	if (!selectSurfaceFormat()) return false;
	if (!selectSurfacePresentMode()) return false;

	vk::SwapchainCreateInfoKHR swapchainCreateInfo(vk::SwapchainCreateFlagsKHR(), vulkan.surface, 2, vulkan.surfaceFormat.format, vulkan.surfaceFormat.colorSpace, { window->getWidth(), window->getHeight() }, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0, nullptr, vk::SurfaceTransformFlagBitsKHR::eIdentity, vk::CompositeAlphaFlagBitsKHR::eOpaque, vk::PresentModeKHR::eFifo);
	if (vulkan.device.createSwapchainKHR(&swapchainCreateInfo, nullptr, &vulkan.swapchain) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create swapchain.");
		return false;
	}

	if (!getImages()) return false;
	if (!createImageViews()) return false;

	return true;
}

bool Swapchain::finalize(const Window *window)
{
	if (!createFramebuffers(window)) return false;
	if (!createCommandBuffers(window)) return false;

	return true;
}