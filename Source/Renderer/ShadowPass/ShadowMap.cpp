#include "ShadowMap.hpp"
#include "../Settings.hpp"

#include <gtc/matrix_transform.hpp>

vk::Image *ShadowMap::createDepthImage(const std::shared_ptr<Context> context)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION, MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION, 1)).setMipLevels(1);
	imageCreateInfo.setArrayLayers(Settings::shadowMapCascadeCount).setFormat(vk::Format::eD32Sfloat).setInitialLayout(vk::ImageLayout::ePreinitialized);
	imageCreateInfo.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *ShadowMap::createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
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
		throw std::runtime_error("Failed to find suitable memory type for shadow map depth image.");
	}

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	context->getDevice()->bindImageMemory(*image, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);
}

vk::ImageView *ShadowMap::createSharedDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2DArray).setFormat(vk::Format::eD32Sfloat);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, Settings::shadowMapCascadeCount));
	auto depthImageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(depthImageView);
}

std::vector<vk::ImageView> *ShadowMap::createDepthImageViews(const std::shared_ptr<Context> context, const vk::Image *image)
{
	auto depthImageViews = std::vector<vk::ImageView>(Settings::shadowMapCascadeCount);
	for (int i = 0; i < Settings::shadowMapCascadeCount; ++i)
	{
		auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(vk::Format::eD32Sfloat);
		imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, i, 1));
		depthImageViews[i] = context->getDevice()->createImageView(imageViewCreateInfo);
	}
	return new std::vector<vk::ImageView>(depthImageViews);
}

std::vector<vk::Framebuffer> *ShadowMap::createFramebuffers(const std::shared_ptr<Context> context, const std::vector<vk::ImageView> *depthImageViews, const vk::RenderPass *renderPass)
{
	auto framebuffers = std::vector<vk::Framebuffer>(Settings::shadowMapCascadeCount);
	for (int i = 0; i < Settings::shadowMapCascadeCount; ++i)
	{
		auto framebufferCreateInfo = vk::FramebufferCreateInfo().setRenderPass(*renderPass).setWidth(MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION).setHeight(MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION);
		framebufferCreateInfo.setAttachmentCount(1).setPAttachments(&depthImageViews->at(i)).setLayers(1);
		framebuffers[i] = context->getDevice()->createFramebuffer(framebufferCreateInfo);
	}
	return new std::vector<vk::Framebuffer>(framebuffers);
}

vk::Sampler *ShadowMap::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge).setAddressModeV(vk::SamplerAddressMode::eClampToEdge).setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
	samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

vk::CommandBuffer *ShadowMap::createCommandBuffer(const std::shared_ptr<Context> context)
{
	vk::CommandBuffer commandBuffer;
	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPoolOnce()).setCommandBufferCount(1);
	if (context->getDevice()->allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to allocate command buffer.");
	}

	return new vk::CommandBuffer(commandBuffer);
}

vk::DescriptorSet *ShadowMap::createSharedDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const ShadowMap *shadowMap)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool->getPool()).setDescriptorSetCount(1).setPSetLayouts(descriptorPool->getShadowMapLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	
	auto shadowMapDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal).setImageView(*shadowMap->sharedDepthImageView).setSampler(*shadowMap->sampler);
	auto shadowMapSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	shadowMapSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&shadowMapDescriptorImageInfo);

	context->getDevice()->updateDescriptorSets(1, &shadowMapSamplerWriteDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

std::vector<vk::DescriptorSet> *ShadowMap::createDescriptorSets(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const ShadowMap *shadowMap)
{
	auto descriptorSets = std::vector<vk::DescriptorSet>(Settings::shadowMapCascadeCount);
	for (int i = 0; i < Settings::shadowMapCascadeCount; ++i)
	{
		auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool->getPool()).setDescriptorSetCount(1).setPSetLayouts(descriptorPool->getShadowMapLayout());
		descriptorSets[i] = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

		auto shadowMapDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal).setImageView(shadowMap->depthImageViews->at(i)).setSampler(*shadowMap->sampler);
		auto shadowMapSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSets[i]).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		shadowMapSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&shadowMapDescriptorImageInfo);

		context->getDevice()->updateDescriptorSets(1, &shadowMapSamplerWriteDescriptorSet, 0, nullptr);
	}
	return new std::vector<vk::DescriptorSet>(descriptorSets);
}

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
ShadowMap::ShadowMap(const std::shared_ptr<Context> context, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> dynamicUniformBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::vector<std::shared_ptr<Model>> *models, uint32_t shadowMapIndex, uint32_t numShadowMaps)
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
ShadowMap::ShadowMap(const std::shared_ptr<Context> context, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> shadowPassDynamicUniformBuffer, const std::shared_ptr<UniformBuffer> geometryPassVertexDynamicUniformBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::vector<std::shared_ptr<Model>> *models, uint32_t shadowMapIndex, uint32_t numShadowMaps)
#endif
{
	this->context = context;
	this->descriptorPool = descriptorPool;

	depthImage = std::unique_ptr<vk::Image, decltype(depthImageDeleter)>(createDepthImage(context), depthImageDeleter);
	depthImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)>(createDepthImageMemory(context, depthImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), depthImageMemoryDeleter);
	sharedDepthImageView = std::unique_ptr<vk::ImageView, decltype(sharedDepthImageViewDeleter)>(createSharedDepthImageView(context, depthImage.get()), sharedDepthImageViewDeleter);
	depthImageViews = std::unique_ptr<std::vector<vk::ImageView>, decltype(depthImageViewsDeleter)>(createDepthImageViews(context, depthImage.get()), depthImageViewsDeleter);
	framebuffers = std::unique_ptr<std::vector<vk::Framebuffer>, decltype(framebuffersDeleter)>(createFramebuffers(context, depthImageViews.get(), shadowPipeline->getRenderPass()), framebuffersDeleter);
	sampler = std::unique_ptr<vk::Sampler, decltype(samplerDeleter)>(createSampler(context), samplerDeleter);

	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPoolOnce()).setCommandBufferCount(1);
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
	context->getDevice()->freeCommandBuffers(*context->getCommandPoolOnce(), 1, &commandBuffer);

	this->commandBuffer = std::unique_ptr<vk::CommandBuffer>(createCommandBuffer(context));

	sharedDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createSharedDescriptorSet(context, descriptorPool, this));
	descriptorSets = std::unique_ptr<std::vector<vk::DescriptorSet>>(createDescriptorSets(context, descriptorPool, this));

	splitDepths.resize(Settings::shadowMapCascadeCount);
	cascadeViewProjectionMatrices.resize(Settings::shadowMapCascadeCount);


	// record command buffer

	commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

	auto renderPassBeginInfo = vk::RenderPassBeginInfo().setRenderPass(*shadowPipeline->getRenderPass());
	renderPassBeginInfo.setRenderArea(vk::Rect2D(vk::Offset2D(), vk::Extent2D(MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION, MK_OPTIMIZATION_SHADOW_MAP_RESOLUTION)));

	vk::ClearValue clearValues[1];
	clearValues[0].depthStencil = vk::ClearDepthStencilValue{ 1.0f, 0 };
	renderPassBeginInfo.setClearValueCount(1).setPClearValues(clearValues);

	this->commandBuffer->begin(commandBufferBeginInfo);

	for (int i = 0; i < Settings::shadowMapCascadeCount; ++i)
	{
		renderPassBeginInfo.setFramebuffer(framebuffers->at(i));
		this->commandBuffer->beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);

		this->commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *shadowPipeline->getPipeline());

		VkDeviceSize offsets[] = { 0 };
		this->commandBuffer->bindVertexBuffers(0, 1, vertexBuffer->getBuffer()->getBuffer(), offsets);
		this->commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer()->getBuffer(), 0, vk::IndexType::eUint32);

		auto pipelineLayout = shadowPipeline->getPipelineLayout();

		auto dynamicOffset = 0u;

		// bind shadow map cascade view projection matrices
//#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
		if (Settings::uniformBufferMode == SETTINGS_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC)
		{
			dynamicOffset = numShadowMaps * context->getUniformBufferDataAlignment() + shadowMapIndex * context->getUniformBufferDataAlignmentLarge();
			this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, dynamicUniformBuffer->getDescriptor(1)->getSet(), 1, &dynamicOffset);
		}
//#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
		else if (Settings::uniformBufferMode == SETTINGS_UNIFORM_BUFFER_MODE_INDIVIDUAL)
		{
			//dynamicOffset = shadowMapIndex * context->getUniformBufferDataAlignment();
			//this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 1, 1, shadowPassDynamicUniformBuffer->getDescriptor()->getSet(), 1, &dynamicOffset);
		}
//#endif

		this->commandBuffer->pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(uint32_t), &i);

		for (uint32_t j = 0; j < models->size(); ++j)
		{
			auto model = models->at(j);

			// bind geometry world matrix
//#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
			if (Settings::uniformBufferMode == SETTINGS_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC)
			{
				dynamicOffset = (numShadowMaps + j) * context->getUniformBufferDataAlignment() + numShadowMaps * context->getUniformBufferDataAlignmentLarge();
				this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, dynamicUniformBuffer->getDescriptor(2)->getSet(), 1, &dynamicOffset);
			}
//#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
			else if (Settings::uniformBufferMode == SETTINGS_UNIFORM_BUFFER_MODE_INDIVIDUAL)
			{
				//dynamicOffset = j * context->getUniformBufferDataAlignment();
				//this->commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, geometryPassVertexDynamicUniformBuffer->getDescriptor()->getSet(), 1, &dynamicOffset);
			}
//#endif

			for (size_t k = 0; k < model->getMeshes()->size(); ++k)
			{
				auto mesh = model->getMeshes()->at(k);
				this->commandBuffer->drawIndexed(mesh->indexCount, 1, mesh->firstIndex, 0, 0);
			}
		}

		this->commandBuffer->endRenderPass();
	}

	this->commandBuffer->end();
}

ShadowMap::~ShadowMap()
{
	// explicitly free the descriptor sets because shadow maps can be rebuild
	context->getDevice()->freeDescriptorSets(*descriptorPool->getPool(), 1, sharedDescriptorSet.get());
	context->getDevice()->freeDescriptorSets(*descriptorPool->getPool(), static_cast<uint32_t>(descriptorSets->size()), descriptorSets->data());
}

void ShadowMap::update(const std::shared_ptr<Camera> camera, const glm::vec3 lightDirection)
{
	float *cascadeSplits = new float[Settings::shadowMapCascadeCount];

	float nearClip = camera->getNearClip();
	float farClip = camera->getFarClip();
	float clipRange = farClip - nearClip;

	float minZ = nearClip;
	float maxZ = nearClip + clipRange;

	float range = maxZ - minZ;
	float ratio = maxZ / minZ;

	// calculate split depths based on view camera frustum
	// based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	for (int i = 0; i < Settings::shadowMapCascadeCount; ++i)
	{
		float p = (i + 1) / static_cast<float>(Settings::shadowMapCascadeCount);
		float log = minZ * std::pow(ratio, p);
		float uniform = minZ + range * p;
		float d = 0.95f * (log - uniform) + uniform;
		cascadeSplits[i] = (d - nearClip) / clipRange;
	}

	// calculate orthographic projection matrix for each cascade
	float lastSplitDist = 0.0;
	for (int i = 0; i < Settings::shadowMapCascadeCount; ++i)
	{
		float splitDist = cascadeSplits[i];

		glm::vec3 frustumCorners[8] =
		{
			glm::vec3(-1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f,  1.0f, -1.0f),
			glm::vec3(1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f, -1.0f, -1.0f),
			glm::vec3(-1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f,  1.0f,  1.0f),
			glm::vec3(1.0f, -1.0f,  1.0f),
			glm::vec3(-1.0f, -1.0f,  1.0f)
		};

		// project frustum corners into world space
		glm::mat4 invCam = glm::inverse((*camera->getProjectionMatrix()) * (*camera->getViewMatrix()));
		for (uint32_t j = 0; j < 8; ++j)
		{
			glm::vec4 invCorner = invCam * glm::vec4(frustumCorners[j], 1.0f);
			frustumCorners[j] = invCorner / invCorner.w;
		}

		for (uint32_t j = 0; j < 4; ++j)
		{
			glm::vec3 dist = frustumCorners[j + 4] - frustumCorners[j];
			frustumCorners[j + 4] = frustumCorners[j] + (dist * splitDist);
			frustumCorners[j] = frustumCorners[j] + (dist * lastSplitDist);
		}

		// get frustum center
		glm::vec3 frustumCenter = glm::vec3(0.0f);
		for (uint32_t j = 0; j < 8; ++j)
		{
			frustumCenter += frustumCorners[j];
		}
		frustumCenter /= 8.0f;

		float radius = 0.0f;
		for (uint32_t j = 0; j < 8; ++j)
		{
			float distance = glm::length(frustumCorners[j] - frustumCenter);
			radius = glm::max(radius, distance);
		}
		radius = std::ceil(radius * 16.0f) / 16.0f;

		glm::vec3 maxExtents = glm::vec3(radius);
		glm::vec3 minExtents = -maxExtents;

		glm::mat4 shadowMapViewMatrix = glm::lookAt(frustumCenter - lightDirection * -minExtents.z, frustumCenter, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 shadowMapProjectionMatrix = glm::ortho(minExtents.x, maxExtents.x, minExtents.y, maxExtents.y, -6500.0f, 6500.0f);
		shadowMapProjectionMatrix[1][1] *= -1.0f;

		// store split distance and matrix in cascade
		splitDepths[i] = (camera->getNearClip() + splitDist * clipRange);
		cascadeViewProjectionMatrices[i] = shadowMapProjectionMatrix * shadowMapViewMatrix;

		lastSplitDist = cascadeSplits[i];
	}
}