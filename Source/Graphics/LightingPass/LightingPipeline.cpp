#include "LightingPipeline.hpp"
#include "..\Buffers.hpp"
#include "..\Shader.hpp"

vk::PipelineLayout *LightingPipeline::createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor)
{
	std::vector<vk::DescriptorSetLayout> setLayouts = { *descriptor->getGeometryBufferDescriptorSetLayout() };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	setLayouts.push_back(*descriptor->getLightDataDescriptorSetLayout());
#endif

	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(static_cast<uint32_t>(setLayouts.size())).setPSetLayouts(setLayouts.data());
	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *LightingPipeline::createPipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout)
{
	Shader vertexShader(context, "Shaders\\LightingPass.vert.spv", vk::ShaderStageFlagBits::eVertex);
	Shader fragmentShader(context, "Shaders\\LightingPass.frag.spv", vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShader.getPipelineShaderStageCreateInfo() };

	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo();

	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleStrip);

	auto viewport = vk::Viewport().setWidth(window->getWidth()).setHeight(window->getHeight()).setMaxDepth(1.0f);
	auto scissor = vk::Rect2D().setExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);

	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eNone).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo();

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	colorBlendAttachmentState.setColorBlendOp(vk::BlendOp::eAdd).setSrcColorBlendFactor(vk::BlendFactor::eOne).setDstColorBlendFactor(vk::BlendFactor::eOne).setBlendEnable(VK_TRUE);
	colorBlendAttachmentState.setAlphaBlendOp(vk::BlendOp::eAdd).setSrcAlphaBlendFactor(vk::BlendFactor::eSrcAlpha).setDstAlphaBlendFactor(vk::BlendFactor::eDstAlpha); // TODO: is this line necessary?
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(1).setPAttachments(&colorBlendAttachmentState);

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

LightingPipeline::LightingPipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const vk::RenderPass *renderPass)
{
	this->context = context;

	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, descriptor), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, context, renderPass, pipelineLayout.get()), pipelineDeleter);
}