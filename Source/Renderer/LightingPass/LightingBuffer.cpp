#include "LightingBuffer.hpp"
#include "../Settings.hpp"

std::vector<vk::Image> *LightingBuffer::createImages(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context)
{
	std::vector<vk::Image> images;
	
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setMipLevels(1).setArrayLayers(1).setInitialLayout(vk::ImageLayout::ePreinitialized);
	imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm).setUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled);

	// fullscale
	imageCreateInfo.setExtent(vk::Extent3D(window->getWidth(), window->getHeight(), 1));
	images.push_back(context->getDevice()->createImage(imageCreateInfo));

	// halfscale
	imageCreateInfo.setExtent(vk::Extent3D(window->getWidth(), window->getHeight(), 1));
	images.push_back(context->getDevice()->createImage(imageCreateInfo));

	return new std::vector<vk::Image>(images);
}

std::vector<vk::DeviceMemory> *LightingBuffer::createImagesMemory(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images, vk::MemoryPropertyFlags memoryPropertyFlags)
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
			throw std::runtime_error("Failed to find suitable memory type for lighting buffer image.");
		}

		auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
		imagesMemory[i] = context->getDevice()->allocateMemory(memoryAllocateInfo);
		context->getDevice()->bindImageMemory(images->at(i), imagesMemory[i], 0);
	}

	return new std::vector<vk::DeviceMemory>(imagesMemory);
}

std::vector<vk::ImageView> *LightingBuffer::createImageViews(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images)
{
	std::vector<vk::ImageView> imageViews;

	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setViewType(vk::ImageViewType::e2D).setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	imageViewCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm);

	for (auto &image : *images)
	{
		imageViewCreateInfo.setImage(image);
		imageViews.push_back(context->getDevice()->createImageView(imageViewCreateInfo));
	}

	return new std::vector<vk::ImageView>(imageViews);
}

vk::RenderPass *LightingBuffer::createRenderPass(const std::shared_ptr<Context> context)
{
	std::vector<vk::AttachmentDescription> attachmentDescriptions;
	
	auto attachmentDescription = vk::AttachmentDescription().setLoadOp(vk::AttachmentLoadOp::eClear).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachmentDescription.setFormat(vk::Format::eR8G8B8A8Unorm).setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	// fullscale
	attachmentDescriptions.push_back(attachmentDescription);

	// halfscale
	attachmentDescriptions.push_back(attachmentDescription);

	std::vector<vk::AttachmentReference> colorAttachmentReferences;

	// fullscale
	colorAttachmentReferences.push_back(vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal));

	// halfscale
	colorAttachmentReferences.push_back(vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eColorAttachmentOptimal));

	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachmentCount(static_cast<uint32_t>(colorAttachmentReferences.size()));
	subpassDescription.setPColorAttachments(colorAttachmentReferences.data());

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

vk::Framebuffer *LightingBuffer::createFramebuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::vector<vk::ImageView> *imageViews, const vk::RenderPass *renderPass)
{
	auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*renderPass).setWidth(window->getWidth()).setHeight(window->getHeight());
	framebufferCreateInfo.setAttachmentCount(static_cast<uint32_t>(imageViews->size())).setPAttachments(imageViews->data()).setLayers(1);
	return new vk::Framebuffer(context->getDevice()->createFramebuffer(framebufferCreateInfo));
}

vk::Sampler *LightingBuffer::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eNearest).setMinFilter(vk::Filter::eNearest).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setAddressModeV(vk::SamplerAddressMode::eClampToEdge).setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

vk::CommandBuffer *LightingBuffer::createCommandBuffer(const std::shared_ptr<Context> context)
{
	vk::CommandBuffer commandBuffer;
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(Settings::reuseCommandBuffers ? *context->getCommandPoolOnce() : *context->getCommandPoolRepeat()).setCommandBufferCount(1);
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return new vk::CommandBuffer(commandBuffer);
}

vk::DescriptorSet *LightingBuffer::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const std::vector<vk::ImageView> *imageViews, const vk::Sampler *sampler)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool->getPool()).setDescriptorSetCount(1).setPSetLayouts(descriptorPool->getLightingBufferLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto fullscaleDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(imageViews->at(0)).setSampler(*sampler);
	auto fullscaleSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	fullscaleSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&fullscaleDescriptorImageInfo);

	auto halfscaleDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(imageViews->at(1)).setSampler(*sampler);
	auto halfscaleSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(1).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	halfscaleSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&halfscaleDescriptorImageInfo);

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { fullscaleSamplerWriteDescriptorSet, halfscaleSamplerWriteDescriptorSet };
	context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

LightingBuffer::LightingBuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool)
{
	this->window = window;
	this->context = context;
	
	images = std::unique_ptr<std::vector<vk::Image>, decltype(imagesDeleter)>(createImages(window, context), imagesDeleter);
	imagesMemory = std::unique_ptr<std::vector<vk::DeviceMemory>, decltype(imagesMemoryDeleter)>(createImagesMemory(context, images.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), imagesMemoryDeleter);
	imageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)>(createImageViews(context, images.get()), imageViewsDeleter);
	
	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	framebuffer = std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)>(createFramebuffer(window, context, imageViews.get(), renderPass.get()), framebufferDeleter);
	sampler = std::unique_ptr<vk::Sampler, decltype(samplerDeleter)>(createSampler(context), samplerDeleter);
	commandBuffer = std::unique_ptr<vk::CommandBuffer>(createCommandBuffer(context));
	descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptorPool, imageViews.get(), sampler.get()));
}

void LightingBuffer::recordCommandBuffers(const std::shared_ptr<LightingPipeline> lightingPipeline, const std::shared_ptr<GeometryBuffer> geometryBuffer, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const vk::DescriptorSet *uniformBufferDescriptorSet, const vk::DescriptorSet *shadowMapCascadesViewProjectionMatricesDescriptorSet, const vk::DescriptorSet *shadowMapCascadeSplitsDescriptorSet, const vk::DescriptorSet *lightWorldMatrixDescriptorSet, const vk::DescriptorSet *lightDataDescriptorSet, const std::vector<std::shared_ptr<Light>> *lights, uint32_t numShadowMaps, uint32_t numModels, const std::shared_ptr<Model> unitQuadModel, const std::shared_ptr<Model> unitSphereModel)
{
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	std::array<float, 4> clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	std::array<vk::ClearValue, 2> clearValues = { vk::ClearColorValue(clearColor), vk::ClearColorValue(clearColor) };

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*renderPass);
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(window->getWidth(), window->getHeight())));
	renderPassBeginInfo.setClearValueCount(static_cast<uint32_t>(clearValues.size())).setPClearValues(clearValues.data());

	commandBuffer->begin(commandBufferBeginInfo);

	commandBuffer->resetQueryPool(*context->getQueryPool(), 4, 2);
	commandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eTopOfPipe, *context->getQueryPool(), 4);

	renderPassBeginInfo.setFramebuffer(*framebuffer);
	commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *lightingPipeline->getPipeline());

	VkDeviceSize offsets[] = { 0 };
	commandBuffer->bindVertexBuffers(0, 1, vertexBuffer->getBuffer()->getBuffer(), offsets);
	commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer()->getBuffer(), 0, vk::IndexType::eUint32);

	auto pipelineLayout = lightingPipeline->getPipelineLayout();

	commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 2, 1, geometryBuffer->getDescriptorSet(), 0, nullptr);
	commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, uniformBufferDescriptorSet, 0, nullptr);

	uint32_t shadowMapIndex = 0;
	for (uint32_t j = 0; j < lights->size(); ++j)
	{
		const auto light = lights->at(j);

		if (light->shadowMap)
		{
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 3, 1, light->shadowMap->getSharedDescriptorSet(), 0, nullptr);

			uint32_t dynamicOffset = 0;
			if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
			{
				dynamicOffset = numShadowMaps * context->getUniformBufferDataAlignment() + shadowMapIndex * context->getUniformBufferDataAlignmentLarge();
			}
			else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
			{
				dynamicOffset = shadowMapIndex * context->getUniformBufferDataAlignmentLarge();
			}

			// bind shadow map cascade view projection matrices
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 5, 1, shadowMapCascadesViewProjectionMatricesDescriptorSet, 1, &dynamicOffset);

			// bind shadow map cascade splits
			dynamicOffset = shadowMapIndex * context->getUniformBufferDataAlignment();
			commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 6, 1, shadowMapCascadeSplitsDescriptorSet, 1, &dynamicOffset);

			++shadowMapIndex;
		}

		uint32_t dynamicOffset = 0;
		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			dynamicOffset = (numShadowMaps + numModels + j) * context->getUniformBufferDataAlignment() + numShadowMaps * context->getUniformBufferDataAlignmentLarge();
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			dynamicOffset = j * context->getUniformBufferDataAlignment();
		}

		// bind light world matrix
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, lightWorldMatrixDescriptorSet, 1, &dynamicOffset);

		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			dynamicOffset = (numShadowMaps + numModels + static_cast<uint32_t>(lights->size()) + j) * context->getUniformBufferDataAlignment() + numShadowMaps * context->getUniformBufferDataAlignmentLarge();
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			dynamicOffset = j * context->getUniformBufferDataAlignment();
		}

		// bind light data
		commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 4, 1, lightDataDescriptorSet, 1, &dynamicOffset);

		if (light->type == LightType::Directional)
		{
			auto mesh = unitQuadModel->getMeshes()->at(0);
			commandBuffer->drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
		}
		else
		{
			auto mesh = unitSphereModel->getMeshes()->at(0);
			commandBuffer->drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
		}
	}

	commandBuffer->endRenderPass();

	commandBuffer->writeTimestamp(vk::PipelineStageFlagBits::eBottomOfPipe, *context->getQueryPool(), 5);

	commandBuffer->end();
}