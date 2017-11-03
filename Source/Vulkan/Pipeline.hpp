#pragma once

#include "Texture.hpp"

// TODO: temporary here
extern UniformBufferObject uniformBufferObject;
extern std::array<glm::mat4, 3> pushConstants;

class Pipeline
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorSetLayout *createDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::function<void(vk::DescriptorSetLayout*)> descriptorSetLayoutDeleter = [this](vk::DescriptorSetLayout *descripterSetLayout) { if (context->getDevice()) context->getDevice()->destroyDescriptorSetLayout(*descripterSetLayout); };
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> descriptorSetLayout;

	static vk::DescriptorPool *createDescriptorPool(const std::shared_ptr<Context> context);
	std::function<void(vk::DescriptorPool*)> descriptorPoolDeleter = [this](vk::DescriptorPool *descriptorPool) { if (context->getDevice()) context->getDevice()->destroyDescriptorPool(*descriptorPool); };
	std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)> descriptorPool;

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Texture> texture, const vk::DescriptorSetLayout *descriptorSetLayout, const vk::DescriptorPool* descriptorPool);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

	static vk::RenderPass *createRenderPass(const std::shared_ptr<Context> context);
	std::function<void(vk::RenderPass*)> renderPassDeleter = [this](vk::RenderPass *renderPass) { if (context->getDevice()) context->getDevice()->destroyRenderPass(*renderPass); };
	std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)> renderPass;

	static vk::PipelineLayout *createPipelineLayout(const std::shared_ptr<Context> context, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::function<void(vk::PipelineLayout*)> pipelineLayoutDeleter = [this](vk::PipelineLayout *pipelineLayout) { if (context->getDevice()) context->getDevice()->destroyPipelineLayout(*pipelineLayout); };
	std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)> pipelineLayout;

	static vk::Pipeline *createPipeline(const std::shared_ptr<Window> window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, const std::shared_ptr<Context> context);
	std::function<void(vk::Pipeline*)> pipelineDeleter = [this](vk::Pipeline *pipeline) { if (context->getDevice()) context->getDevice()->destroyPipeline(*pipeline); };
	std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)> pipeline;

public:
	Pipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Texture> texture);

	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); };
	vk::RenderPass *getRenderPass() const { return renderPass.get(); }
	vk::PipelineLayout *getPipelineLayout() const { return pipelineLayout.get(); }
	vk::Pipeline *getPipeline() const { return pipeline.get(); }
};