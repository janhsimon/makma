#pragma once

#include "ShadowPipeline.hpp"
#include "..\Buffers\UniformBuffer.hpp"
#include "..\Model.hpp"


class ShadowMap
{
private:
	std::shared_ptr<Context> context;

	static vk::Image *createDepthImage(const std::shared_ptr<Context> context);
	std::function<void(vk::Image*)> depthImageDeleter = [this](vk::Image *depthImage) { if (context->getDevice()) context->getDevice()->destroyImage(*depthImage); };
	std::unique_ptr<vk::Image, decltype(depthImageDeleter)> depthImage;

	static vk::DeviceMemory *createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> depthImageMemoryDeleter = [this](vk::DeviceMemory *depthImageMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*depthImageMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)> depthImageMemory;

	static vk::ImageView *createDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image);
	std::function<void(vk::ImageView*)> depthImageViewDeleter = [this](vk::ImageView *depthImageView) { if (context->getDevice()) context->getDevice()->destroyImageView(*depthImageView); };
	std::unique_ptr<vk::ImageView, decltype(depthImageViewDeleter)> depthImageView;

	static vk::Framebuffer *createFramebuffer(const std::shared_ptr<Context> context, const vk::ImageView *depthImageView, const vk::RenderPass *renderPass);
	std::function<void(vk::Framebuffer*)> framebufferDeleter = [this](vk::Framebuffer *framebuffer) { if (context->getDevice()) context->getDevice()->destroyFramebuffer(*framebuffer); };
	std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)> framebuffer;

	static vk::Sampler *createSampler(const std::shared_ptr<Context> context);
	std::function<void(vk::Sampler*)> samplerDeleter = [this](vk::Sampler *sampler) { if (context->getDevice()) context->getDevice()->destroySampler(*sampler); };
	std::unique_ptr<vk::Sampler, decltype(samplerDeleter)> sampler;

	static vk::CommandBuffer *createCommandBuffer(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::CommandBuffer> commandBuffer;

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const ShadowMap *shadowMap);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

public:
	ShadowMap(const std::shared_ptr<Context> context, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> uniformBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::vector<std::shared_ptr<Model>> *models, uint32_t shadowMapIndex, uint32_t numShadowMaps);

	glm::mat4 getViewProjectionMatrix(const glm::vec3 position) const;

	vk::CommandBuffer *getCommandBuffer() const { return commandBuffer.get(); }
	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); }
};
