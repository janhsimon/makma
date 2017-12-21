#pragma once

#include "..\Descriptor.hpp"
#include "..\Texture.hpp"

class ShadowPipeline
{
private:
	std::shared_ptr<Context> context;

	static vk::PipelineLayout *createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor);
	std::function<void(vk::PipelineLayout*)> pipelineLayoutDeleter = [this](vk::PipelineLayout *pipelineLayout) { if (context->getDevice()) context->getDevice()->destroyPipelineLayout(*pipelineLayout); };
	std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)> pipelineLayout;

	static vk::Pipeline *createPipeline(const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, const std::shared_ptr<Context> context);
	std::function<void(vk::Pipeline*)> pipelineDeleter = [this](vk::Pipeline *pipeline) { if (context->getDevice()) context->getDevice()->destroyPipeline(*pipeline); };
	std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)> pipeline;

public:
	ShadowPipeline(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const vk::RenderPass *renderPass);

	vk::PipelineLayout *getPipelineLayout() const { return pipelineLayout.get(); }
	vk::Pipeline *getPipeline() const { return pipeline.get(); }
};