#pragma once

#include "LightingPipeline.hpp"
#include "../Model.hpp"
#include "../GeometryPass/GeometryBuffer.hpp"
#include "../../Light.hpp"

class LightingBuffer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;
	std::shared_ptr<Model> unitQuadModel;

	static std::vector<vk::Image> *createImages(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context);
	std::function<void(std::vector<vk::Image>*)> imagesDeleter = [this](std::vector<vk::Image> *images) { if (context->getDevice()) { for (auto &image : *images) context->getDevice()->destroyImage(image); } };
	std::unique_ptr<std::vector<vk::Image>, decltype(imagesDeleter)> images;

	static std::vector<vk::DeviceMemory> *createImagesMemory(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(std::vector<vk::DeviceMemory>*)> imagesMemoryDeleter = [this](std::vector<vk::DeviceMemory> *depthImageMemory) { if (context->getDevice()) { for (auto &imageMemory : *imagesMemory) context->getDevice()->freeMemory(imageMemory); } };
	std::unique_ptr<std::vector<vk::DeviceMemory>, decltype(imagesMemoryDeleter)> imagesMemory;

	static std::vector<vk::ImageView> *createImageViews(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images);
	std::function<void(std::vector<vk::ImageView>*)> imageViewsDeleter = [this](std::vector<vk::ImageView> *imageViews) { if (context->getDevice()) { for (auto &imageView : *imageViews) context->getDevice()->destroyImageView(imageView); } };
	std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)> imageViews;

	static vk::RenderPass *createRenderPass(const std::shared_ptr<Context> context);
	std::function<void(vk::RenderPass*)> renderPassDeleter = [this](vk::RenderPass *renderPass) { if (context->getDevice()) context->getDevice()->destroyRenderPass(*renderPass); };
	std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)> renderPass;

	static vk::Framebuffer *createFramebuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::vector<vk::ImageView> *imageViews, const vk::RenderPass *renderPass);
	std::function<void(vk::Framebuffer*)> framebufferDeleter = [this](vk::Framebuffer *framebuffer) { if (context->getDevice()) context->getDevice()->destroyFramebuffer(*framebuffer); };
	std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)> framebuffer;

	static vk::Sampler *createSampler(const std::shared_ptr<Context> context);
	std::function<void(vk::Sampler*)> samplerDeleter = [this](vk::Sampler *sampler) { if (context->getDevice()) context->getDevice()->destroySampler(*sampler); };
	std::unique_ptr<vk::Sampler, decltype(samplerDeleter)> sampler;

	static vk::CommandBuffer *createCommandBuffer(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::CommandBuffer> commandBuffer;

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const std::vector<vk::ImageView> *imageViews, const vk::Sampler *sampler);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

public:
	LightingBuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool);

	void recordCommandBuffers(const std::shared_ptr<LightingPipeline> lightingPipeline, const std::shared_ptr<GeometryBuffer> geometryBuffer, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const vk::DescriptorSet *uniformBufferDescriptorSet, const vk::DescriptorSet *shadowMapCascadesViewProjectionMatricesDescriptorSet, const vk::DescriptorSet *shadowMapCascadeSplitsDescriptorSet, const vk::DescriptorSet *lightWorldMatrixDescriptorSet, const vk::DescriptorSet *lightDataDescriptorSet, const std::shared_ptr<std::vector<std::shared_ptr<Light>>> *lights, uint32_t numShadowMaps, uint32_t numModels, const std::shared_ptr<Model> unitQuadModel, const std::shared_ptr<Model> unitSphereModel);

	vk::RenderPass *getRenderPass() const { return renderPass.get(); }
	vk::CommandBuffer *getCommandBuffer() const { return commandBuffer.get(); }
	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); }
};