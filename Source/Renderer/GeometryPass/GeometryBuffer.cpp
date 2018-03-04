#include "GeometryBuffer.hpp"

std::vector<vk::Image> *GeometryBuffer::createImages(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context)
{
	std::vector<vk::Image> images;
	
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(window->getWidth(), window->getHeight(), 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setInitialLayout(vk::ImageLayout::ePreinitialized).setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	// albedo rgb and occlusion
	imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);
	images.push_back(context->getDevice()->createImage(imageCreateInfo));

	// world-space normal xy, metallic and roughness
	imageCreateInfo.setFormat(vk::Format::eR16G16B16A16Sfloat); // TODO: vk::Format::eR8G8B8A8Unorm
	images.push_back(context->getDevice()->createImage(imageCreateInfo));

	return new std::vector<vk::Image>(images);
}

std::vector<vk::DeviceMemory> *GeometryBuffer::createImagesMemory(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto imagesMemory = std::vector<vk::DeviceMemory>(images->size());
	for (size_t i = 0; i < imagesMemory.size(); ++i)
	{
		auto memoryRequirements = context->getDevice()->getImageMemoryRequirements(images->at(i));
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
			throw std::runtime_error("Failed to find suitable memory type for geometry buffer image.");
		}

		auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
		imagesMemory[i] = context->getDevice()->allocateMemory(memoryAllocateInfo);
		context->getDevice()->bindImageMemory(images->at(i), imagesMemory[i], 0);
	}

	return new std::vector<vk::DeviceMemory>(imagesMemory);
}

std::vector<vk::ImageView> *GeometryBuffer::createImageViews(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images)
{
	std::vector<vk::ImageView> imageViews;

	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setViewType(vk::ImageViewType::e2D).setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	// albedo rgb and occlusion
	imageViewCreateInfo.setImage(images->at(0)).setFormat(vk::Format::eR8G8B8A8Unorm);
	imageViews.push_back(context->getDevice()->createImageView(imageViewCreateInfo));

	// world-space normal xy, metallic and roughness
	imageViewCreateInfo.setImage(images->at(1)).setFormat(vk::Format::eR16G16B16A16Sfloat); // TODO: vk::Format::eR8G8B8A8Unorm
	imageViews.push_back(context->getDevice()->createImageView(imageViewCreateInfo));

	return new std::vector<vk::ImageView>(imageViews);
}

vk::Image *GeometryBuffer::createDepthImage(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(window->getWidth(), window->getHeight(), 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setFormat(vk::Format::eD32Sfloat).setInitialLayout(vk::ImageLayout::ePreinitialized).setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *GeometryBuffer::createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
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

vk::ImageView *GeometryBuffer::createDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eD32Sfloat);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	auto depthImageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(depthImageView);
}

vk::RenderPass *GeometryBuffer::createRenderPass(const std::shared_ptr<Context> context)
{
	std::vector<vk::AttachmentDescription> attachmentDescriptions;
	
	auto attachmentDescription = vk::AttachmentDescription().setLoadOp(vk::AttachmentLoadOp::eClear).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);

	// albedo rgb and occlusion
	attachmentDescription.setFormat(vk::Format::eR8G8B8A8Unorm).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	attachmentDescriptions.push_back(attachmentDescription);

	// world-space normal xy, metallic and roughness
	attachmentDescription.setFormat(vk::Format::eR16G16B16A16Sfloat).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // TODO: vk::Format::eR8G8B8A8Unorm
	attachmentDescriptions.push_back(attachmentDescription);

	// depth
	attachmentDescription.setFormat(vk::Format::eD32Sfloat).setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	attachmentDescriptions.push_back(attachmentDescription);

	std::vector<vk::AttachmentReference> colorAttachmentReferences;

	// albedo rgb and occlusion
	colorAttachmentReferences.push_back(vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal));

	// world-space normal xy, metallic and roughness
	colorAttachmentReferences.push_back(vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eColorAttachmentOptimal));

	auto depthAttachmentReference = vk::AttachmentReference().setAttachment(2).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachmentCount(static_cast<uint32_t>(colorAttachmentReferences.size()));
	subpassDescription.setPColorAttachments(colorAttachmentReferences.data()).setPDepthStencilAttachment(&depthAttachmentReference);

	std::vector<vk::SubpassDependency> subpassDependencies;

	auto subpassDependency = vk::SubpassDependency().setSrcSubpass(VK_SUBPASS_EXTERNAL).setDstSubpass(0).setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe).setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead).setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);
	subpassDependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(subpassDependency);

	subpassDependency.setSrcSubpass(0).setDstSubpass(VK_SUBPASS_EXTERNAL).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstStageMask(vk::PipelineStageFlagBits::eBottomOfPipe);
	subpassDependency.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite).setDstAccessMask(vk::AccessFlagBits::eMemoryRead);
	subpassDependencies.push_back(subpassDependency);

	auto renderPassCreateInfo = vk::RenderPassCreateInfo().setAttachmentCount(static_cast<uint32_t>(attachmentDescriptions.size())).setPAttachments(attachmentDescriptions.data());
	renderPassCreateInfo.setSubpassCount(1).setPSubpasses(&subpassDescription).setDependencyCount(static_cast<uint32_t>(subpassDependencies.size())).setPDependencies(subpassDependencies.data());
	auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
	return new vk::RenderPass(renderPass);
}

vk::Framebuffer *GeometryBuffer::createFramebuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::vector<vk::ImageView> *imageViews, const vk::ImageView *depthImageView, const vk::RenderPass *renderPass)
{
	std::vector<vk::ImageView> attachments(*imageViews);
	attachments.push_back(*depthImageView);

	auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*renderPass).setWidth(window->getWidth()).setHeight(window->getHeight());
	framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(attachments.size())).setPAttachments(attachments.data()).setLayers(1);
	return new vk::Framebuffer(context->getDevice()->createFramebuffer(framebufferCreateInfo));
}

vk::Sampler *GeometryBuffer::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setAddressModeV(vk::SamplerAddressMode::eClampToEdge).setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

vk::CommandBuffer *GeometryBuffer::createCommandBuffer(const std::shared_ptr<Context> context)
{
	vk::CommandBuffer commandBuffer;
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return new vk::CommandBuffer(commandBuffer);
}

vk::DescriptorSet *GeometryBuffer::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const std::vector<vk::ImageView> *imageViews, const vk::ImageView *depthImageView, const vk::Sampler *sampler)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool->getPool()).setDescriptorSetCount(1).setPSetLayouts(descriptorPool->getGeometryBufferLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	// albedo rgb and occlusion
	auto albedoDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(imageViews->at(0)).setSampler(*sampler);
	auto albedoSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	albedoSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&albedoDescriptorImageInfo);

	// world-space normal xy, metallic and roughness
	auto normalDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(imageViews->at(1)).setSampler(*sampler);
	auto normalSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(1).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&normalDescriptorImageInfo);

	// depth
	auto depthDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal).setImageView(*depthImageView).setSampler(*sampler);
	auto depthSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(2).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	depthSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&depthDescriptorImageInfo);

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { albedoSamplerWriteDescriptorSet, normalSamplerWriteDescriptorSet, depthSamplerWriteDescriptorSet };
	context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

GeometryBuffer::GeometryBuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool)
{
	this->window = window;
	this->context = context;
	
	images = std::unique_ptr<std::vector<vk::Image>, decltype(imagesDeleter)>(createImages(window, context), imagesDeleter);
	imagesMemory = std::unique_ptr<std::vector<vk::DeviceMemory>, decltype(imagesMemoryDeleter)>(createImagesMemory(context, images.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), imagesMemoryDeleter);
	imageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)>(createImageViews(context, images.get()), imageViewsDeleter);

	depthImage = std::unique_ptr<vk::Image, decltype(depthImageDeleter)>(createDepthImage(window, context), depthImageDeleter);
	depthImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)>(createDepthImageMemory(context, depthImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), depthImageMemoryDeleter);
	depthImageView = std::unique_ptr<vk::ImageView, decltype(depthImageViewDeleter)>(createDepthImageView(context, depthImage.get()), depthImageViewDeleter);

	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);

	framebuffer = std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)>(createFramebuffer(window, context, imageViews.get(), depthImageView.get(), renderPass.get()), framebufferDeleter);

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

	descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptorPool, imageViews.get(), depthImageView.get(), sampler.get()));
}

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
void GeometryBuffer::recordCommandBuffer(const std::shared_ptr<GeometryPipeline> geometryPipeline, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> uniformBuffer, const std::shared_ptr<UniformBuffer> dynamicUniformBuffer, const std::vector<std::shared_ptr<Model>> *models, uint32_t numShadowMaps)
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
void GeometryBuffer::recordCommandBuffer(const std::shared_ptr<GeometryPipeline> geometryPipeline, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> geometryPassVertexDynamicUniformBuffer, const std::shared_ptr<UniformBuffer> geometryPassVertexUniformBuffer, const std::vector<std::shared_ptr<Model>> *models, uint32_t numShadowMaps)
#endif
{
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	std::array<vk::ClearValue, 3> clearValues = { vk::ClearColorValue(clearColor), vk::ClearColorValue(clearColor), vk::ClearDepthStencilValue(1.0f, 0) };

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*renderPass);
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight())));
	renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size())).setPClearValues(clearValues.data());

	commandBuffer->begin(commandBufferBeginInfo);

	renderPassBeginInfo.setFramebuffer(*framebuffer);
	commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *geometryPipeline->getPipeline());

	VkDeviceSize offsets[] = { 0 };
	commandBuffer->bindVertexBuffers(0, 1, vertexBuffer->getBuffer()->getBuffer(), offsets);
	commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer()->getBuffer(), 0, vk::IndexType::eUint32);

	auto pipelineLayout = geometryPipeline->getPipelineLayout();

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	// bind camera view projection matrix
	commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, uniformBuffer->getDescriptor(0)->getSet(), 0, nullptr);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, geometryPassVertexUniformBuffer->getDescriptor()->getSet(), 0, nullptr);
#endif

	for (uint32_t i = 0; i < models->size(); ++i)
	{
		auto model = models->at(i);

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
		// bind geometry world matrix
		auto dynamicOffset = (numShadowMaps + i) * context->getUniformBufferDataAlignment() + numShadowMaps * context->getUniformBufferDataAlignmentLarge();
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, dynamicUniformBuffer->getDescriptor(2)->getSet(), 1, &dynamicOffset);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
		uint32_t dynamicOffset = i * context->getUniformBufferDataAlignment();
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, geometryPassVertexDynamicUniformBuffer->getDescriptor()->getSet(), 1, &dynamicOffset);
#endif

		for (size_t j = 0; j < model->getMeshes()->size(); ++j)
		{
			auto mesh = model->getMeshes()->at(j);
			auto material = mesh->material.get();
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 2, 1, material->getDescriptorSet(), 0, nullptr);
			commandBuffer->drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
		}
	}

	commandBuffer->endRenderPass();
	commandBuffer->end();
}