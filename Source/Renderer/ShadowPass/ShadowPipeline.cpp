#include "ShadowPipeline.hpp"
#include "..\Buffers.hpp"
#include "..\Shader.hpp"

vk::PipelineLayout *ShadowPipeline::createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor)
{
	std::vector<vk::DescriptorSetLayout> setLayouts;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	setLayouts.push_back(*descriptor->getWorldMatrixDescriptorSetLayout());
	setLayouts.push_back(*descriptor->getViewProjectionMatrixDescriptorSetLayout());
#endif

	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(static_cast<uint32_t>(setLayouts.size())).setPSetLayouts(setLayouts.data());

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	auto pushConstantRange = vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(*buffers->getPushConstants()));
	pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
	pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
#endif

	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *ShadowPipeline::createPipeline(const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, std::shared_ptr<Context> context)
{
#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	Shader vertexShader(context, "Shaders\\GeometryPassPushConstants.vert.spv", vk::ShaderStageFlagBits::eVertex);
#else
	Shader vertexShader(context, "Shaders\\ShadowPassUBO.vert.spv", vk::ShaderStageFlagBits::eVertex);
#endif

	Shader fragmentShader(context, "Shaders\\ShadowPass.frag.spv", vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShader.getPipelineShaderStageCreateInfo() };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, position));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position };
	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vertexInputBindingDescription);
	vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInputAttributeDescriptions.size()));
	vertexInputStateCreateInfo.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions.data());

	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto viewport = vk::Viewport().setWidth(4096).setHeight(4096).setMaxDepth(1.0f);
	auto scissor = vk::Rect2D().setExtent(vk::Extent2D(4096, 4096));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);

	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eBack).setFrontFace(vk::FrontFace::eCounterClockwise).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

	auto depthStenctilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(true).setDepthWriteEnable(true).setDepthCompareOp(vk::CompareOp::eLess);

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setColorWriteMask(vk::ColorComponentFlagBits::eR);
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(1).setPAttachments(&colorBlendAttachmentState);

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPDepthStencilState(&depthStenctilStateCreateInfo);
	pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

ShadowPipeline::ShadowPipeline(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const vk::RenderPass *renderPass)
{
	this->context = context;

	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, buffers, descriptor), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(renderPass, pipelineLayout.get(), context), pipelineDeleter);
}