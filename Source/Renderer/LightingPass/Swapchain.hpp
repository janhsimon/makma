#pragma once

#include "LightingPipeline.hpp"
#include "..\Model.hpp"
#include "..\Buffers\IndexBuffer.hpp"
#include "..\Buffers\VertexBuffer.hpp"
#include "..\Buffers\UniformBuffer.hpp"
#include "..\GeometryPass\GeometryBuffer.hpp"
#include "..\..\Camera.hpp"
#include "..\..\Light.hpp"

class Swapchain
{
private:
	std::shared_ptr<Context> context;
	std::shared_ptr<Window> window;
	std::shared_ptr<Model> unitQuadModel, unitSphereModel;

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
	Swapchain(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Model> unitQuadModel, const std::shared_ptr<Model> unitSphereModel);

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	void recordCommandBuffers(const std::shared_ptr<LightingPipeline> lightingPipeline, const std::shared_ptr<GeometryBuffer> geometryBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> uniformBuffer, const std::shared_ptr<UniformBuffer> dynamicUniformBuffer, const std::vector<std::shared_ptr<Light>> *lights, uint32_t numShadowMaps, uint32_t numModels);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	void recordCommandBuffers(const std::shared_ptr<LightingPipeline> lightingPipeline, const std::shared_ptr<GeometryBuffer> geometryBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> shadowPassDynamicUniformBuffer, const std::shared_ptr<UniformBuffer> lightingPassVertexDynamicUniformBuffer, const std::shared_ptr<UniformBuffer> lightingPassVertexUniformBuffer, const std::shared_ptr<UniformBuffer> lightingPassFragmentDynamicUniformBuffer, const std::vector<std::shared_ptr<Light>> *lights, uint32_t numShadowMaps, uint32_t numModels);
#endif

	vk::SwapchainKHR *getSwapchain() const { return swapchain.get(); }
	vk::RenderPass *getRenderPass() const { return renderPass.get(); }
	vk::CommandBuffer *getCommandBuffer(uint32_t index) const { return &commandBuffers.get()->at(index); }
};