#pragma once

#include "..\Texture.hpp"

class ShadowPipeline
{
private:
	std::shared_ptr<Context> context;

	static vk::RenderPass *createRenderPass(const std::shared_ptr<Context> context);
	std::function<void(vk::RenderPass*)> renderPassDeleter = [this](vk::RenderPass *renderPass) { if (context->getDevice()) context->getDevice()->destroyRenderPass(*renderPass); };
	std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)> renderPass;

	static vk::PipelineLayout *createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts);
	std::function<void(vk::PipelineLayout*)> pipelineLayoutDeleter = [this](vk::PipelineLayout *pipelineLayout) { if (context->getDevice()) context->getDevice()->destroyPipelineLayout(*pipelineLayout); };
	std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)> pipelineLayout;

	static vk::Pipeline *createPipeline(const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, const std::shared_ptr<Context> context);
	std::function<void(vk::Pipeline*)> pipelineDeleter = [this](vk::Pipeline *pipeline) { if (context->getDevice()) context->getDevice()->destroyPipeline(*pipeline); };
	std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)> pipeline;

public:
	ShadowPipeline(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts);

	vk::RenderPass *getRenderPass() const { return renderPass.get(); }
	vk::PipelineLayout *getPipelineLayout() const { return pipelineLayout.get(); }
	vk::Pipeline *getPipeline() const { return pipeline.get(); }
};