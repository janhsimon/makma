#pragma once

#include "CompositePipeline.hpp"
#include "../Model.hpp"
#include "../Buffers/IndexBuffer.hpp"
#include "../Buffers/VertexBuffer.hpp"
#include "../Buffers/UniformBuffer.hpp"
#include "../LightingPass/LightingBuffer.hpp"
#include "../../Camera.hpp"
#include "../../Light.hpp"

class Swapchain
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;

	static vk::SwapchainKHR *createSwapchain(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context);
	std::function<void(vk::SwapchainKHR*)> swapchainDeleter = [this](vk::SwapchainKHR *swapchain) { if (context->getDevice()) context->getDevice()->destroySwapchainKHR(*swapchain); };
	std::unique_ptr<vk::SwapchainKHR, decltype(swapchainDeleter)> swapchain;

	static std::vector<vk::Image> *getImages(const std::shared_ptr<Context> context, const vk::SwapchainKHR *swapchain);
	std::unique_ptr<std::vector<vk::Image>> images;

	static std::vector<vk::ImageView> *createImageViews(const std::shared_ptr<Context> context, const std::vector<vk::Image> *images);
	std::function<void(std::vector<vk::ImageView>*)> imageViewsDeleter = [this](std::vector<vk::ImageView> *imageViews) { if (context->getDevice()) { for (auto &imageView : *imageViews) context->getDevice()->destroyImageView(imageView); } };
	std::unique_ptr<std::vector<vk::ImageView>, decltype(imageViewsDeleter)> imageViews;

	static vk::RenderPass *createRenderPass(const std::shared_ptr<Context> context);
	std::function<void(vk::RenderPass*)> renderPassDeleter = [this](vk::RenderPass *renderPass) { if (context->getDevice()) context->getDevice()->destroyRenderPass(*renderPass); };
	std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)> renderPass;

	static std::vector<vk::Framebuffer> *createFramebuffers(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const vk::RenderPass *renderPass, const std::vector<vk::ImageView> *imageViews);
	std::function<void(std::vector<vk::Framebuffer>*)> framebuffersDeleter = [this](std::vector<vk::Framebuffer> *framebuffers) { if (context->getDevice()) { for (auto &framebuffer : *framebuffers) context->getDevice()->destroyFramebuffer(framebuffer); } };
	std::unique_ptr<std::vector<vk::Framebuffer>, decltype(framebuffersDeleter)> framebuffers;

	static std::vector<vk::CommandBuffer> *createCommandBuffers(const std::shared_ptr<Context> context, const std::vector<vk::Framebuffer> *framebuffers);
	std::unique_ptr<std::vector<vk::CommandBuffer>> commandBuffers;

public:
	Swapchain(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context);

	void recordCommandBuffers(const std::shared_ptr<CompositePipeline> compositePipeline, const std::shared_ptr<LightingBuffer> lightingBuffer, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<Model> unitQuadModel);

	vk::SwapchainKHR *getSwapchain() const { return swapchain.get(); }
	vk::RenderPass *getRenderPass() const { return renderPass.get(); }
	vk::CommandBuffer *getCommandBuffer(const uint32_t index) const { return &commandBuffers.get()->at(index); }
};