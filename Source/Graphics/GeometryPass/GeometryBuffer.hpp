#pragma once

#include "GeometryPipeline.hpp"
#include "..\Buffers.hpp"
#include "..\Model.hpp"
#include "..\..\Logic\Camera.hpp"

class GeometryBuffer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;

	static std::vector<vk::Image> *createImages(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context);
	std::function<void(std::vector<vk::Image>*)> imagesDeleter = [this](std::vector<vk::Image> *images) { if (context->getDevice()) { for (auto &image : *images) context->getDevice()->destroyImage(image); } };
	std::unique_ptr<std::vector<vk::Image>, decltype(imagesDeleter)> images;

	static std::vector<vk::DeviceMemory> *createImagesMemory(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(std::vector<vk::DeviceMemory>*)> imagesMemoryDeleter = [this](std::vector<vk::DeviceMemory> *depthImageMemory) { if (context->getDevice()) { for (auto &imageMemory : *imagesMemory) context->getDevice()->freeMemory(imageMemory); } };
	std::unique_ptr<std::vector<vk::DeviceMemory>, decltype(imagesMemoryDeleter)> imagesMemory;

	static std::vector<vk::ImageView> *createImageViews(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images);
	std::function<void(std::vector<vk::ImageView>*)> imageViewsDeleter = [this](std::vector<vk::ImageView> *imageViews) { if (context->getDevice()) { for (auto &imageView : *imageViews) context->getDevice()->destroyImageView(imageView); } };
	std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)> imageViews;

	static vk::Image *createDepthImage(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context);
	std::function<void(vk::Image*)> depthImageDeleter = [this](vk::Image *depthImage) { if (context->getDevice()) context->getDevice()->destroyImage(*depthImage); };
	std::unique_ptr<vk::Image, decltype(depthImageDeleter)> depthImage;

	static vk::DeviceMemory *createDepthImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> depthImageMemoryDeleter = [this](vk::DeviceMemory *depthImageMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*depthImageMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)> depthImageMemory;

	static vk::ImageView *createDepthImageView(const std::shared_ptr<Context> context, const vk::Image *image);
	std::function<void(vk::ImageView*)> depthImageViewDeleter = [this](vk::ImageView *depthImageView) { if (context->getDevice()) context->getDevice()->destroyImageView(*depthImageView); };
	std::unique_ptr<vk::ImageView, decltype(depthImageViewDeleter)> depthImageView;

	static vk::RenderPass *createRenderPass(const std::shared_ptr<Context> context);
	std::function<void(vk::RenderPass*)> renderPassDeleter = [this](vk::RenderPass *renderPass) { if (context->getDevice()) context->getDevice()->destroyRenderPass(*renderPass); };
	std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)> renderPass;

	static vk::Framebuffer *createFramebuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::vector<vk::ImageView> *imageViews, const vk::ImageView *depthImageView, const vk::RenderPass *renderPass);
	std::function<void(vk::Framebuffer*)> framebufferDeleter = [this](vk::Framebuffer *framebuffer) { if (context->getDevice()) context->getDevice()->destroyFramebuffer(*framebuffer); };
	std::unique_ptr<vk::Framebuffer, decltype(framebufferDeleter)> framebuffer;

	static vk::Sampler *createSampler(const std::shared_ptr<Context> context);
	std::function<void(vk::Sampler*)> samplerDeleter = [this](vk::Sampler *sampler) { if (context->getDevice()) context->getDevice()->destroySampler(*sampler); };
	std::unique_ptr<vk::Sampler, decltype(samplerDeleter)> sampler;

	static vk::CommandBuffer *createCommandBuffer(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::CommandBuffer> commandBuffer;

public:
	GeometryBuffer(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context);

	void recordCommandBuffer(const std::shared_ptr<GeometryPipeline> geometryPipeline, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const std::vector<Model*> *models, const std::shared_ptr<Camera> camera);

	vk::RenderPass *getRenderPass() const { return renderPass.get(); }
	vk::CommandBuffer *getCommandBuffer(uint32_t index) const { return commandBuffer.get(); }
};