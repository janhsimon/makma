#include "Swapchain.hpp"

vk::SwapchainKHR *Swapchain::createSwapchain(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context)
{
	// FIFO is guaranteed to be available and requires 2 images in the swapchain
	auto selectedPresentMode = vk::PresentModeKHR::eFifo;
	uint32_t imageCount = 2;

	auto presentModes = context->getPhysicalDevice()->getSurfacePresentModesKHR(*context->getSurface());
	for (auto &presentMode : presentModes)
	{
		if (presentMode == vk::PresentModeKHR::eMailbox)
		// prefer MAILBOX for triple buffering and add an extra swapchain image
		{
			imageCount = 3;
			selectedPresentMode = presentMode;
			break;
		}
	}
	
	// TODO: this could also be moved into the Context::createPhysicalDevice() function, at the end, and then it could
	// store the surface format and presentation mode stuff and so on, which we would then simply read here from the context
	auto surfaceCapabilities = context->getPhysicalDevice()->getSurfaceCapabilitiesKHR(*context->getSurface());
	if ((surfaceCapabilities.minImageCount > imageCount || surfaceCapabilities.maxImageCount < imageCount) && surfaceCapabilities.maxImageCount != 0)
	// maxImageCount == 0 means there are no restrictions
	{
		throw std::runtime_error("The physical device does not meet the surface capability requirements.");
	}

	bool surfaceFormatFound = false;
	auto surfaceFormats = context->getPhysicalDevice()->getSurfaceFormatsKHR(*context->getSurface());
	if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined)
	// undefined as the only result means there are no preferences
	{
		surfaceFormatFound = true;
	}
	else
	{
		for (auto &surfaceFormat : surfaceFormats)
		{
			if (surfaceFormat.format == vk::Format::eB8G8R8A8Unorm && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
			{
				surfaceFormatFound = true;
				break;
			}
		}
	}

	if (!surfaceFormatFound)
	{
		throw std::runtime_error("Failed to find suitable surface format for physical device.");
	}

	auto swapchainCreateInfo = vk::SwapchainCreateInfoKHR().setSurface(*context->getSurface()).setMinImageCount(imageCount).setImageFormat(vk::Format::eB8G8R8A8Unorm);
	swapchainCreateInfo.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear).setImageExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	swapchainCreateInfo.setImageArrayLayers(1).setImageUsage(vk::ImageUsageFlagBits::eColorAttachment).setPresentMode(selectedPresentMode);
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

vk::Image *Swapchain::createDepthImage(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(window->getWidth(), window->getHeight(), 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setFormat(vk::Format::eD32Sfloat).setInitialLayout(vk::ImageLayout::ePreinitialized).setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *Swapchain::createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto memoryRequirements = context->getDevice()->getImageMemoryRequirements(*image);
	auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	bool foundMatch = false;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
		{
			memoryTypeIndex = i;
			foundMatch = true;
			break;
		}
	}

	if (!foundMatch)
	{
		throw std::runtime_error("Failed to find suitable memory type for depth image.");
	}

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	context->getDevice()->bindImageMemory(*image, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);
}

vk::ImageView *Swapchain::createDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eD32Sfloat);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	auto depthImageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(depthImageView);
}

std::vector<vk::Framebuffer> *Swapchain::createFramebuffers(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Pipeline> pipeline, const std::vector<vk::ImageView> *imageViews, const vk::ImageView *depthImageView)
{
	auto framebuffers = std::vector<vk::Framebuffer>(imageViews->size());
	for (size_t i = 0; i < framebuffers.size(); ++i)
	{
		std::vector<vk::ImageView> attachments = { imageViews->at(i), *depthImageView };
		auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*pipeline->getRenderPass()).setWidth(window->getWidth()).setHeight(window->getHeight());
		framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size())).setPAttachments(attachments.data()).setLayers(1);
		framebuffers[i] = context->getDevice()->createFramebuffer(framebufferCreateInfo);
	}

	return new std::vector<vk::Framebuffer>(framebuffers);
}

std::vector<vk::CommandBuffer> *Swapchain::createCommandBuffers(const std::shared_ptr<Context> context, const std::vector<vk::Framebuffer> *framebuffers)
{
	auto commandBuffers = std::vector<vk::CommandBuffer>(framebuffers->size());
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size()));
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, commandBuffers.data()) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffers.");
	}

	return new std::vector<vk::CommandBuffer>(commandBuffers);
}

Swapchain::Swapchain(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context)
{
	this->context = context;
	this->window = window;

	swapchain = std::unique_ptr<vk::SwapchainKHR, decltype(swapchainDeleter)>(createSwapchain(window, context), swapchainDeleter);
	images = std::unique_ptr<std::vector<vk::Image>>(getImages(context, swapchain.get()));
	imageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)>(createImageViews(context, images.get()), imageViewsDeleter);
	
	depthImage = std::unique_ptr<vk::Image, decltype(depthImageDeleter)>(createDepthImage(window, context), depthImageDeleter);
	depthImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)>(createDepthImageMemory(context, depthImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), depthImageMemoryDeleter);
	depthImageView = std::unique_ptr<vk::ImageView, decltype(depthImageViewDeleter)>(createDepthImageView(context, depthImage.get()), depthImageViewDeleter);

	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	auto commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	auto barrier = vk::ImageMemoryBarrier().setOldLayout(vk::ImageLayout::eUndefined).setNewLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal).setImage(*depthImage);
	barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	barrier.setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer.end();
	auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);
	context->getQueue().submit({ submitInfo }, nullptr);
	context->getQueue().waitIdle();
	context->getDevice()->freeCommandBuffers(*context->getCommandPool(), 1, &commandBuffer);
}

void Swapchain::createFramebuffers(const std::shared_ptr<Pipeline> pipeline)
{
	framebuffers = std::unique_ptr<std::vector<vk::Framebuffer>, decltype(framebuffersDeleter)>(createFramebuffers(window, context, pipeline, imageViews.get(), depthImageView.get()), framebuffersDeleter);
}

void Swapchain::createCommandBuffers()
{
	commandBuffers = std::unique_ptr<std::vector<vk::CommandBuffer>>(createCommandBuffers(context, framebuffers.get()));
}

void Swapchain::recordCommandBuffers(const std::shared_ptr<Pipeline> pipeline, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const std::vector<Model*> *models, const std::shared_ptr<Camera> camera)
{
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	std::array<float, 4> clearColor = { 1.0f, 0.8f, 0.4f, 1.0f };
	std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(clearColor), vk::ClearDepthStencilValue(1.0f, 0) };

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*pipeline->getRenderPass());
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight())));
	renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size())).setPClearValues(clearValues.data());

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	buffers->getPushConstants()->at(1) = *camera.get()->getViewMatrix();
	buffers->getPushConstants()->at(2) = *camera.get()->getProjectionMatrix();
	buffers->getPushConstants()->at(2)[1][1] *= -1.0f;
#endif

	for (size_t i = 0; i < commandBuffers->size(); ++i)
	{
		auto commandBuffer = commandBuffers->at(i);
		commandBuffer.begin(commandBufferBeginInfo);

		renderPassBeginInfo.setFramebuffer(framebuffers->at(i));
		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline->getPipeline());

		VkDeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, 1, buffers->getVertexBuffer(), offsets);
		commandBuffer.bindIndexBuffer(*buffers->getIndexBuffer(), 0, vk::IndexType::eUint32);

		auto pipelineLayout = pipeline->getPipelineLayout();
		
#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 2, 1, descriptor->getViewProjectionMatrixDescriptorSet(), 0, nullptr);
#endif

		for (uint32_t j = 0; j < models->size(); ++j)
		{
			auto model = models->at(j);

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
			buffers->getPushConstants()->at(0) = model->getWorldMatrix();
			commandBuffer.pushConstants(*pipeline->getPipelineLayout(), vk::ShaderStageFlagBits::eVertex, 0, sizeof(*buffers->getPushConstants()), buffers->getPushConstants()->data());
#else
			uint32_t dynamicOffset = j * static_cast<uint32_t>( buffers->getDynamicAlignment());
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, descriptor->getWorldMatrixDescriptorSet(), 1, &dynamicOffset);
#endif

			for (size_t k = 0; k < model->getMeshes()->size(); ++k)
			{
				auto mesh = model->getMeshes()->at(k);
				auto material = mesh->material.get();
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, material->getDescriptorSet(), 0, nullptr);
				commandBuffer.drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
			}
		}

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}
}