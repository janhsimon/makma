#pragma once

#include "Descriptor.hpp"
#include "Model.hpp"
#include "ShadowPass\ShadowPipeline.hpp"

enum LightType
{
	Directional,
	Point
};

class Light
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

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const Light *light);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

public:
	LightType type;
	glm::vec3 position;
	glm::vec3 color;
	float range, intensity, specularPower;
	bool castShadows;
	
	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, float specularPower, bool castShadows);

	void finalize(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::vector<std::shared_ptr<Model>> *models);

	vk::CommandBuffer *getCommandBuffer() const { return commandBuffer.get(); }
	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); }
};
