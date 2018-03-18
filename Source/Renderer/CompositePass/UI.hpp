#pragma once

#include "../Buffers/Buffer.hpp"
#include "../Buffers/DescriptorPool.hpp"
#include "../../Camera.hpp"
#include "../../Input.hpp"

#include <glm.hpp>
#include <imgui.h>

class UI
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;

	ImGuiContext *imGuiContext;

	int32_t vertexCount, indexCount;
	void *vertexBufferMemory, *indexBufferMemory;

	std::unique_ptr<Buffer> vertexBuffer, indexBuffer;

	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };

	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };

	static vk::Image *createFontImage(const std::shared_ptr<Context> context, uint32_t width, uint32_t height);
	std::function<void(vk::Image*)> fontImageDeleter = [this](vk::Image *fontImage) { if (context->getDevice()) context->getDevice()->destroyImage(*fontImage); };
	std::unique_ptr<vk::Image, decltype(fontImageDeleter)> fontImage;

	static vk::DeviceMemory *createFontImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> fontImageMemoryDeleter = [this](vk::DeviceMemory *fontImageMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*fontImageMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(fontImageMemoryDeleter)> fontImageMemory;

	static vk::ImageView *createFontImageView(const std::shared_ptr<Context> context, const vk::Image *image);
	std::function<void(vk::ImageView*)> fontImageViewDeleter = [this](vk::ImageView *fontImageView) { if (context->getDevice()) context->getDevice()->destroyImageView(*fontImageView); };
	std::unique_ptr<vk::ImageView, decltype(fontImageViewDeleter)> fontImageView;

	static vk::Sampler *createSampler(const std::shared_ptr<Context> context);
	std::function<void(vk::Sampler*)> samplerDeleter = [this](vk::Sampler *sampler) { if (context->getDevice()) context->getDevice()->destroySampler(*sampler); };
	std::unique_ptr<vk::Sampler, decltype(samplerDeleter)> sampler;

	static vk::PipelineLayout *createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts);
	std::function<void(vk::PipelineLayout*)> pipelineLayoutDeleter = [this](vk::PipelineLayout *pipelineLayout) { if (context->getDevice()) context->getDevice()->destroyPipelineLayout(*pipelineLayout); };
	std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)> pipelineLayout;

	static vk::Pipeline *createPipeline(const std::shared_ptr<Window> window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, const std::shared_ptr<Context> context);
	std::function<void(vk::Pipeline*)> pipelineDeleter = [this](vk::Pipeline *pipeline) { if (context->getDevice()) context->getDevice()->destroyPipeline(*pipeline); };
	std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)> pipeline;

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const vk::ImageView *fontImageView, const vk::Sampler *sampler);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

public:
	static int shadowMapCascadeCount;

	UI(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, std::vector<vk::DescriptorSetLayout> setLayouts, vk::RenderPass *renderPass);
	~UI() { if (imGuiContext) ImGui::DestroyContext(imGuiContext); }

	void update(const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera, float delta);
	void render(const vk::CommandBuffer *commandBuffer);
};