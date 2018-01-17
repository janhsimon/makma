#include "Light.hpp"

vk::Image *Light::createDepthImage(const std::shared_ptr<Context> context)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(4096, 4096, 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setFormat(vk::Format::eD32Sfloat).setInitialLayout(vk::ImageLayout::ePreinitialized).setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *Light::createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
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

vk::ImageView *Light::createDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eD32Sfloat);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
	auto depthImageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(depthImageView);
}

vk::Framebuffer *Light::createFramebuffer(const std::shared_ptr<Context> context, const vk::ImageView *depthImageView, const vk::RenderPass *renderPass)
{
	auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*renderPass).setWidth(4096).setHeight(4096).setAttachmentCount(1).setPAttachments(depthImageView).setLayers(1);
	return new vk::Framebuffer(context->getDevice()->createFramebuffer(framebufferCreateInfo));
}

vk::Sampler *Light::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setAddressModeV(vk::SamplerAddressMode::eClampToEdge).setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

vk::CommandBuffer *Light::createCommandBuffer(const std::shared_ptr<Context> context)
{
	vk::CommandBuffer commandBuffer;
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return new vk::CommandBuffer(commandBuffer);
}

vk::DescriptorSet *Light::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const Light *light)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptor->getDescriptorPool()).setDescriptorSetCount(1).setPSetLayouts(descriptor->getShadowMapMaterialDescriptorSetLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto shadowMapDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*light->depthImageView).setSampler(*light->sampler);
	auto shadowMapSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	shadowMapSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&shadowMapDescriptorImageInfo);

	context->getDevice()->updateDescriptorSets(1, &shadowMapSamplerWriteDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, float specularPower, bool castShadows)
{
	this->type = type;
	this->position = position;
	this->color = color;
	this->intensity = intensity;
	this->specularPower = specularPower;
	this->castShadows = castShadows;

	setRange(range);
}

void Light::finalize(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::vector<std::shared_ptr<Model>> *models, uint32_t shadowMapIndex, uint32_t numShadowMaps)
{
	if (!castShadows)
	{
		return;
	}

	this->context = context;

	depthImage = std::unique_ptr<vk::Image, decltype(depthImageDeleter)>(createDepthImage(context), depthImageDeleter);
	depthImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)>(createDepthImageMemory(context, depthImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), depthImageMemoryDeleter);
	depthImageView = std::unique_ptr<vk::ImageView, decltype(depthImageViewDeleter)>(createDepthImageView(context, depthImage.get()), depthImageViewDeleter);

	framebuffer = std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)>(createFramebuffer(context, depthImageView.get(), shadowPipeline->getRenderPass()), framebufferDeleter);

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

	descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptor, this));


	// record command buffer

	commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*shadowPipeline->getRenderPass());
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(4096, 4096)));

	vk::ClearValue clearValues[1];
	clearValues[0].depthStencil = { 1.0f, 0 };
	renderPassBeginInfo.setClearValueCount(1).setPClearValues(clearValues);

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS
	buffers->getPushConstants()->at(1) = *camera.get()->getViewMatrix();
	buffers->getPushConstants()->at(2) = *camera.get()->getProjectionMatrix();
	buffers->getPushConstants()->at(2)[1][1] *= -1.0f;
#endif

	this->commandBuffer->begin(commandBufferBeginInfo);

	renderPassBeginInfo.setFramebuffer(*framebuffer);
	this->commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

	this->commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *shadowPipeline->getPipeline());

	VkDeviceSize offsets[] = { 0 };
	this->commandBuffer->bindVertexBuffers(0, 1, buffers->getVertexBuffer(), offsets);
	this->commandBuffer->bindIndexBuffer(*buffers->getIndexBuffer(), 0, vk::IndexType::eUint32);

	auto pipelineLayout = shadowPipeline->getPipelineLayout();

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	// shadow map view * proj
	uint32_t dynamicOffset = shadowMapIndex * static_cast<uint32_t>(buffers->getDataAlignment());
	this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, descriptor->getDynamicUniformBufferDescriptorSet(), 1, &dynamicOffset);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	uint32_t dynamicOffset = shadowMapIndex * static_cast<uint32_t>(buffers->getDataAlignment());
	this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, descriptor->getShadowPassDynamicDescriptorSet(), 1, &dynamicOffset);
#endif

	for (uint32_t i = 0; i < models->size(); ++i)
	{
		auto model = models->at(i);

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS
		buffers->getPushConstants()->at(0) = model->getWorldMatrix();
		commandBuffer->pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(*buffers->getPushConstants()), buffers->getPushConstants()->data());
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
		// geometry world matrix
		dynamicOffset = (numShadowMaps + i) * static_cast<uint32_t>(buffers->getDataAlignment());
		this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, descriptor->getDynamicUniformBufferDescriptorSet(), 1, &dynamicOffset);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
		dynamicOffset = i * static_cast<uint32_t>(buffers->getDataAlignment());
		this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, descriptor->getGeometryPassVertexDynamicDescriptorSet(), 1, &dynamicOffset);
#endif

		for (size_t j = 0; j < model->getMeshes()->size(); ++j)
		{
			auto mesh = model->getMeshes()->at(j);
			this->commandBuffer->drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
		}
	}

	this->commandBuffer->endRenderPass();
	this->commandBuffer->end();
}

void Light::setRange(float range)
{
	this->range = range;
	this->scale = glm::vec3(range);
}