#include "ShadowBuffer.hpp"

vk::Image *ShadowBuffer::createDepthImage(const std::shared_ptr<Context> context)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(4096, 4096, 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setFormat(vk::Format::eD32Sfloat).setInitialLayout(vk::ImageLayout::ePreinitialized).setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *ShadowBuffer::createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
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

vk::ImageView *ShadowBuffer::createDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eD32Sfloat);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	auto depthImageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(depthImageView);
}

vk::RenderPass *ShadowBuffer::createRenderPass(const std::shared_ptr<Context> context)
{
	auto attachmentDescription = vk::AttachmentDescription().setLoadOp(vk::AttachmentLoadOp::eClear).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachmentDescription.setFormat(vk::Format::eD32Sfloat).setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	auto depthAttachmentReference = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setPDepthStencilAttachment(&depthAttachmentReference);

	/*
	std::vector<vk::SubpassDependency> subpassDependencies;

	auto subpassDependency = vk::SubpassDependency().setSrcSubpass(VK_SUBPASS_EXTERNAL).setDstSubpass(0).setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe).setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead).setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
	subpassDependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(subpassDependency);

	subpassDependency.setSrcSubpass(0).setDstSubpass(VK_SUBPASS_EXTERNAL).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	subpassDependency.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
	subpassDependencies.push_back(subpassDependency);
	*/

	auto renderPassCreateInfo = vk::RenderPassCreateInfo().setAttachmentCount(1).setPAttachments(&attachmentDescription).setSubpassCount(1).setPSubpasses(&subpassDescription);
	auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
	return new vk::RenderPass(renderPass);
}

vk::Framebuffer *ShadowBuffer::createFramebuffer(const std::shared_ptr<Context> context, const vk::ImageView *depthImageView, const vk::RenderPass *renderPass)
{
	auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*renderPass).setWidth(4096).setHeight(4096).setAttachmentCount(1).setPAttachments(depthImageView).setLayers(1);
	return new vk::Framebuffer(context->getDevice()->createFramebuffer(framebufferCreateInfo));
}

vk::Sampler *ShadowBuffer::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setAddressModeV(vk::SamplerAddressMode::eClampToEdge).setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

vk::CommandBuffer *ShadowBuffer::createCommandBuffer(const std::shared_ptr<Context> context)
{
	vk::CommandBuffer commandBuffer;
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return new vk::CommandBuffer(commandBuffer);
}

vk::DescriptorSet *ShadowBuffer::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const vk::Sampler *sampler)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptor->getDescriptorPool()).setDescriptorSetCount(1).setPSetLayouts(descriptor->getGeometryBufferDescriptorSetLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	return new vk::DescriptorSet(descriptorSet);
}

ShadowBuffer::ShadowBuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor)
{
	this->window = window;
	this->context = context;
	this->descriptor = descriptor;

	/*
	images = std::unique_ptr<std::vector<vk::Image>, decltype(imagesDeleter)>(createImages(window, context), imagesDeleter);
	imagesMemory = std::unique_ptr<std::vector<vk::DeviceMemory>, decltype(imagesMemoryDeleter)>(createImagesMemory(context, images.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), imagesMemoryDeleter);
	imageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)>(createImageViews(context, images.get()), imageViewsDeleter);
	*/

	depthImage = std::unique_ptr<vk::Image, decltype(depthImageDeleter)>(createDepthImage(context), depthImageDeleter);
	depthImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)>(createDepthImageMemory(context, depthImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), depthImageMemoryDeleter);
	depthImageView = std::unique_ptr<vk::ImageView, decltype(depthImageViewDeleter)>(createDepthImageView(context, depthImage.get()), depthImageViewDeleter);
	
	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);

	framebuffer = std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)>(createFramebuffer(context, depthImageView.get(), renderPass.get()), framebufferDeleter);

	sampler = std::unique_ptr<vk::Sampler, decltype(samplerDeleter)>(createSampler(context), samplerDeleter);

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

	this->commandBuffer = std::unique_ptr<vk::CommandBuffer>(createCommandBuffer(context));

	descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptor, sampler.get()));
}

void ShadowBuffer::recordCommandBuffer(const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::shared_ptr<Buffers> buffers, const std::vector<Model*> *models, const std::shared_ptr<Camera> camera)
{
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	std::array<vk::ClearValue, 4> clearValues = { vk::ClearColorValue(clearColor), vk::ClearColorValue(clearColor), vk::ClearColorValue(clearColor), vk::ClearDepthStencilValue(1.0f, 0) };

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*renderPass);
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight())));
	renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size())).setPClearValues(clearValues.data());

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	buffers->getPushConstants()->at(1) = *camera.get()->getViewMatrix();
	buffers->getPushConstants()->at(2) = *camera.get()->getProjectionMatrix();
	buffers->getPushConstants()->at(2)[1][1] *= -1.0f;
#endif

	commandBuffer->begin(commandBufferBeginInfo);

	renderPassBeginInfo.setFramebuffer(*framebuffer);
	commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *shadowPipeline->getPipeline());

	VkDeviceSize offsets[] = { 0 };
	commandBuffer->bindVertexBuffers(0, 1, buffers->getVertexBuffer(), offsets);
	commandBuffer->bindIndexBuffer(*buffers->getIndexBuffer(), 0, vk::IndexType::eUint32);

	auto pipelineLayout = shadowPipeline->getPipelineLayout();

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 2, 1, descriptor->getViewProjectionMatrixDescriptorSet(), 0, nullptr);
#endif

	for (uint32_t i = 0; i < models->size(); ++i)
	{
		auto model = models->at(i);

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
		buffers->getPushConstants()->at(0) = model->getWorldMatrix();
		commandBuffer->pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(*buffers->getPushConstants()), buffers->getPushConstants()->data());
#else
		uint32_t dynamicOffset = i * static_cast<uint32_t>(buffers->getWorldDataAlignment());
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, descriptor->getWorldMatrixDescriptorSet(), 1, &dynamicOffset);
#endif

		for (size_t j = 0; j < model->getMeshes()->size(); ++j)
		{
			auto mesh = model->getMeshes()->at(j);
			commandBuffer->drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
		}
	}

	commandBuffer->endRenderPass();
	commandBuffer->end();
}