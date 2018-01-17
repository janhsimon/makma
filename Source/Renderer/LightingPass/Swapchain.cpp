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

vk::RenderPass *Swapchain::createRenderPass(const std::shared_ptr<Context> context)
{
	auto colorAttachmentDescription = vk::AttachmentDescription().setFormat(vk::Format::eB8G8R8A8Unorm).setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAttachmentDescription.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	auto colorAttachmentReference = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachmentCount(1).setPColorAttachments(&colorAttachmentReference);

	auto subpassDependency = vk::SubpassDependency().setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	auto renderPassCreateInfo = vk::RenderPassCreateInfo().setAttachmentCount(1).setPAttachments(&colorAttachmentDescription);
	renderPassCreateInfo.setSubpassCount(1).setPSubpasses(&subpassDescription).setDependencyCount(1).setPDependencies(&subpassDependency);
	auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
	return new vk::RenderPass(renderPass);
}

std::vector<vk::Framebuffer> *Swapchain::createFramebuffers(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const vk::RenderPass *renderPass, const std::vector<vk::ImageView> *imageViews)
{
	auto framebuffers = std::vector<vk::Framebuffer>(imageViews->size());
	for (size_t i = 0; i < framebuffers.size(); ++i)
	{
		auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*renderPass).setWidth(window->getWidth()).setHeight(window->getHeight());
		framebufferCreateInfo.setAttachmentCount(1).setPAttachments(&imageViews->at(i)).setLayers(1);
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
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return new std::vector<vk::CommandBuffer>(commandBuffers);
}

Swapchain::Swapchain(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Model> unitQuadModel, const std::shared_ptr<Model> unitSphereModel)
{
	this->context = context;
	this->window = window;
	this->unitQuadModel = unitQuadModel;
	this->unitSphereModel = unitSphereModel;

	swapchain = std::unique_ptr<vk::SwapchainKHR, decltype(swapchainDeleter)>(createSwapchain(window, context), swapchainDeleter);
	images = std::unique_ptr<std::vector<vk::Image>>(getImages(context, swapchain.get()));
	imageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)>(createImageViews(context, images.get()), imageViewsDeleter);

	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	framebuffers = std::unique_ptr<std::vector<vk::Framebuffer>, decltype(framebuffersDeleter)>(createFramebuffers(window, context, renderPass.get(), imageViews.get()), framebuffersDeleter);
	commandBuffers = std::unique_ptr<std::vector<vk::CommandBuffer>>(createCommandBuffers(context, framebuffers.get()));
}

void Swapchain::recordCommandBuffers(const std::shared_ptr<LightingPipeline> lightingPipeline, const std::shared_ptr<GeometryBuffer> geometryBuffer, const std::shared_ptr<Descriptor> descriptor, const std::shared_ptr<Buffers> buffers, const std::vector<std::shared_ptr<Light>> *lights, uint32_t numShadowMaps, uint32_t numModels)
{
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	std::vector<vk::ClearValue> clearValues = { vk::ClearColorValue(clearColor), vk::ClearDepthStencilValue(1.0f, 0) };

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*renderPass);
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight())));
	renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size())).setPClearValues(clearValues.data());

	for (size_t i = 0; i < commandBuffers->size(); ++i)
	{
		auto commandBuffer = commandBuffers->at(i);
		commandBuffer.begin(commandBufferBeginInfo);

		renderPassBeginInfo.setFramebuffer(framebuffers->at(i));
		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipeline());

		VkDeviceSize offsets[] = { 0 };
		commandBuffer.bindVertexBuffers(0, 1, buffers->getVertexBuffer(), offsets);
		commandBuffer.bindIndexBuffer(*buffers->getIndexBuffer(), 0, vk::IndexType::eUint32);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 2, 1, geometryBuffer->getDescriptorSet(), 0, nullptr);

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
		// camera view * proj matrix and globals
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 1, 1, descriptor->getUniformBufferDescriptorSet(), 0, nullptr);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 1, 1, descriptor->getLightingPassVertexDescriptorSet(), 0, nullptr);
#endif

		uint32_t shadowMapIndex = 0;
		for (uint32_t j = 0; j < lights->size(); ++j)
		{
			const auto light = lights->at(j);

			if (light->castShadows)
			{
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 3, 1, light->getShadowMapDescriptorSet(), 0, nullptr);

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
				// shadow map view * proj
				auto dynamicOffset = (shadowMapIndex++) * static_cast<uint32_t>(buffers->getDataAlignment());
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 5, 1, descriptor->getDynamicUniformBufferDescriptorSet(), 1, &dynamicOffset);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
				auto dynamicOffset = (shadowMapIndex++) * static_cast<uint32_t>(buffers->getDataAlignment());
				commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 5, 1, descriptor->getShadowPassDynamicDescriptorSet(), 1, &dynamicOffset);
#endif
			}

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

			// light world matrix
			auto dynamicOffset = (numShadowMaps + numModels + j) * static_cast<uint32_t>(buffers->getDataAlignment());
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 0, 1, descriptor->getDynamicUniformBufferDescriptorSet(), 1, &dynamicOffset);

			// encoded light data
			dynamicOffset = (numShadowMaps + numModels + static_cast<uint32_t>(lights->size()) + j) * static_cast<uint32_t>(buffers->getDataAlignment());
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 4, 1, descriptor->getDynamicUniformBufferDescriptorSet(), 1, &dynamicOffset);

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

			auto dynamicOffset = j * static_cast<uint32_t>(buffers->getDataAlignment());
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 0, 1, descriptor->getLightingPassVertexDynamicDescriptorSet(), 1, &dynamicOffset);

			dynamicOffset = j * static_cast<uint32_t>(buffers->getDataAlignment());
			commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipelineLayout(), 4, 1, descriptor->getLightingPassFragmentDynamicDescriptorSet(), 1, &dynamicOffset);

#endif

			if (light->type == LightType::Directional)
			{
				auto mesh = unitQuadModel->getMeshes()->at(0);
				commandBuffer.drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
			}
			else if (light->type == LightType::Point)
			{
				auto mesh = unitSphereModel->getMeshes()->at(0);
				commandBuffer.drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
			}
		}

		commandBuffer.endRenderPass();
		commandBuffer.end();
	}
}