#include "Swapchain.hpp"

vk::SwapchainKHR *Swapchain::createSwapchain(const Window *window, const std::shared_ptr<Context> context)
{
	// TODO: this could also be moved into the Context::createPhysicalDevice() function, at the end, and then it could
	// store the surface format and presentation mode stuff and so on, which we would then simply read here from the context
	auto surfaceCapabilities = context->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*context->getSurface());
	if ((surfaceCapabilities.minImageCount > 2 || surfaceCapabilities.maxImageCount < 2) && surfaceCapabilities.maxImageCount != 0)
	// maxImageCount == 0 means there are no restrictions
	{
		throw std::runtime_error("The physical device does not meet the surface capability requirements.");
	}

	auto surfaceFormats = context->getPhysicalDevice()->getSurfaceFormatsKHR(*context->getSurface());
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format != vk::Format::eUndefined)
	// undefined as the only result means there are no preferences
	{
		throw std::runtime_error("Failed to find suitable surface format for physical device.");
	}

	bool surfaceFormatFound = false;
	for (auto &surfaceFormat : surfaceFormats)
	{
		if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			surfaceFormatFound = true;
			break;
		}
	}

	if (!surfaceFormatFound)
	{
		throw std::runtime_error("Failed to find suitable surface format for physical device.");
	}

	auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR().setSurface(*context->getSurface()).setMinImageCount(2).setImageFormat(vk::Format::eB8G8R8A8Unorm);
	swapchainCreateInfo.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear).setImageExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	swapchainCreateInfo.setImageArrayLayers(1).setImageUsage(vk::ImageUsageFlagBits::eColorAttachment).setPresentMode(vk::PresentModeKHR::eFifo);
	auto swapchain = context->getDevice()->createSwapchainKHR(swapchainCreateInfo);
	return new vk::SwapchainKHR(swapchain);
}

std::vector<vk::Image> *Swapchain::getImages(const std::shared_ptr<Context> context, const vk::SwapchainKHR *swapchain)
{
	auto swapchainImages = context->getDevice()->getSwapchainImagesKHR(*swapchain);
	return new std::vector<vk::Image>(swapchainImages);
}

std::vector<vk::ImageView> *Swapchain::createImageViews(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images)
{
	auto imageViews = std::vector<vk::ImageView>(images->size());
	for (size_t i = 0; i < imageViews.size(); ++i)
	{
		auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(images->at(i)).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eB8G8R8A8Unorm);
		imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
		imageViews[i] = context->getDevice()->createImageView(imageViewCreateInfo);
	}

	return new std::vector<vk::ImageView>(imageViews);
}

std::vector<vk::Framebuffer> *Swapchain::createFramebuffers(const Window *window, const std::shared_ptr<Context> context, const std::shared_ptr<Pipeline> pipeline, const std::vector<vk::ImageView> *imageViews)
{
	auto framebuffers = std::vector<vk::Framebuffer>(imageViews->size());
	for (size_t i = 0; i < framebuffers.size(); ++i)
	{
		auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*pipeline->getRenderPass()).setAttachmentCount(1).setPAttachments(&imageViews->at(i));
		framebufferCreateInfo.setWidth(window->getWidth()).setHeight(window->getHeight()).setLayers(1);
		framebuffers[i] = context->getDevice()->createFramebuffer(framebufferCreateInfo);
	}

	return new std::vector<vk::Framebuffer>(framebuffers);
}

std::vector<vk::CommandBuffer> *Swapchain::createCommandBuffers(const Window *window, const std::shared_ptr<Context> context, const std::shared_ptr<Pipeline> pipeline, const std::shared_ptr<Buffers> buffers, const std::vector<vk::Framebuffer> *framebuffers)
{
	auto commandBuffers = std::vector<vk::CommandBuffer>(framebuffers->size());
	
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, commandBuffers.data()) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}

	for (size_t i = 0; i < commandBuffers.size(); ++i)
	{
		auto beginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);
		commandBuffers[i].begin(beginInfo);
		
		std::array<float, 4> clearColor = { 1.0f, 0.8f, 0.4f, 1.0f };
		auto clearValue = vk::ClearValue().setColor(clearColor);

		auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*pipeline->getRenderPass()).setFramebuffer(framebuffers->at(i));
		renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight()))).setClearValueCount(1).setPClearValues(&clearValue);
		commandBuffers[i].beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->getPipeline());

		VkDeviceSize offsets[] = { 0 };
		commandBuffers[i].bindVertexBuffers(0, 1, buffers->getVertexBuffer(), offsets);

		commandBuffers[i].draw(static_cast<uint32_t>(vertices.size()), 1, 0, 0);
		vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

		/*
		vk::DeviceSize offsets[] = { 0 };
		vulkan.swapchainCommandBuffers[i].bindVertexBuffers(0, 1, &vulkan.vertexBuffer, offsets);
		vulkan.swapchainCommandBuffers[i].bindIndexBuffer(vulkan.indexBuffer, 0, vk::IndexType::eUint32);

		vulkan.swapchainCommandBuffers[i].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
		*/

		commandBuffers[i].endRenderPass();
		commandBuffers[i].end();
	}

	return new std::vector<vk::CommandBuffer>(commandBuffers);
}

Swapchain::Swapchain(const Window *window, const std::shared_ptr<Context> context)
{
	this->context = context;

	swapchain = std::unique_ptr<vk::SwapchainKHR, decltype(swapchainDeleter)>(createSwapchain(window, context), swapchainDeleter);
	images = std::unique_ptr<std::vector<vk::Image>>(getImages(context, swapchain.get()));
	imageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)>(createImageViews(context, images.get()), imageViewsDeleter);
}

void Swapchain::finalize(const Window *window, const std::shared_ptr<Pipeline> pipeline, const std::shared_ptr<Buffers> buffers)
{
	framebuffers = std::unique_ptr<std::vector<vk::Framebuffer>, decltype(framebuffersDeleter)>(createFramebuffers(window, context, pipeline, imageViews.get()), framebuffersDeleter);
	commandBuffers = std::unique_ptr<std::vector<vk::CommandBuffer>>(createCommandBuffers(window, context, pipeline, buffers, framebuffers.get()));
}