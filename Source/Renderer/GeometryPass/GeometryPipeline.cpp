#include "GeometryPipeline.hpp"
#include "..\Buffers.hpp"
#include "..\Shader.hpp"

vk::PipelineLayout *GeometryPipeline::createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor)
{
	std::vector<vk::DescriptorSetLayout> setLayouts = { *descriptor->getMaterialDescriptorSetLayout() };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	setLayouts.push_back(*descriptor->getGeometryPassVertexDynamicDescriptorSetLayout());
	setLayouts.push_back(*descriptor->getGeometryPassVertexDescriptorSetLayout());
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

vk::Pipeline *GeometryPipeline::createPipeline(const std::shared_ptr<Window> window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, std::shared_ptr<Context> context)
{
#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	Shader vertexShader(context, "Shaders\\GeometryPassPushConstants.vert.spv", vk::ShaderStageFlagBits::eVertex);
#else
	Shader vertexShader(context, "Shaders\\GeometryPassUBO.vert.spv", vk::ShaderStageFlagBits::eVertex);
#endif

	Shader fragmentShader(context, "Shaders\\GeometryPass.frag.spv", vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShader.getPipelineShaderStageCreateInfo() };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, position));
	auto texCoord = vk::VertexInputAttributeDescription().setLocation(1).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(Vertex, texCoord));
	auto normal = vk::VertexInputAttributeDescription().setLocation(2).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, normal));
	auto tangent = vk::VertexInputAttributeDescription().setLocation(3).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, tangent));
	auto bitangent = vk::VertexInputAttributeDescription().setLocation(4).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, bitangent));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position, texCoord, normal, tangent, bitangent };
	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vertexInputBindingDescription);
	vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInputAttributeDescriptions.size()));
	vertexInputStateCreateInfo.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions.data());

	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto viewport = vk::Viewport().setWidth(window->getWidth()).setHeight(window->getHeight()).setMaxDepth(1.0f);
	auto scissor = vk::Rect2D().setExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);

	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eBack).setFrontFace(vk::FrontFace::eCounterClockwise).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

	auto depthStenctilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(true).setDepthWriteEnable(true).setDepthCompareOp(vk::CompareOp::eLess);

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setSrcColorBlendFactor(vk::BlendFactor::eOne).setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = { colorBlendAttachmentState, colorBlendAttachmentState, colorBlendAttachmentState };
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(static_cast<uint32_t>(colorBlendAttachmentStates.size())).setPAttachments(colorBlendAttachmentStates.data());

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPDepthStencilState(&depthStenctilStateCreateInfo);
	pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

GeometryPipeline::GeometryPipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor, const vk::RenderPass *renderPass)
{
	this->context = context;

	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, buffers, descriptor), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass, pipelineLayout.get(), context), pipelineDeleter);
}