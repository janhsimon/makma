#include "ShadowPipeline.hpp"
#include "..\Buffers.hpp"
#include "..\Shader.hpp"

vk::RenderPass *ShadowPipeline::createRenderPass(const std::shared_ptr<Context> context)
{
	auto attachmentDescription = vk::AttachmentDescription().setLoadOp(vk::AttachmentLoadOp::eClear).setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	attachmentDescription.setFormat(vk::Format::eD32Sfloat).setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);
	auto depthAttachmentReference = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setPDepthStencilAttachment(&depthAttachmentReference);

	std::vector<vk::SubpassDependency> subpassDependencies;

	auto subpassDependency = vk::SubpassDependency().setSrcSubpass(VK_SUBPASS_EXTERNAL).setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe).setDstStageMask(vk::PipelineStageFlagBits::eLateFragmentTests);
	subpassDependency.setSrcAccessMask(vk::AccessFlagBits::eShaderRead).setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);
	subpassDependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
	subpassDependencies.push_back(subpassDependency);

	subpassDependency.setSrcStageMask(vk::PipelineStageFlagBits::eLateFragmentTests).setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader).setDstAccessMask(vk::AccessFlagBits::eShaderRead);
	subpassDependencies.push_back(subpassDependency);

	auto renderPassCreateInfo = vk::RenderPassCreateInfo().setAttachmentCount(1).setPAttachments(&attachmentDescription).setSubpassCount(1).setPSubpasses(&subpassDescription);
	renderPassCreateInfo.setDependencyCount(static_cast<uint32_t>(subpassDependencies.size())).setPDependencies(subpassDependencies.data());
	auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
	return new vk::RenderPass(renderPass);
}

vk::PipelineLayout *ShadowPipeline::createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor)
{
	std::vector<vk::DescriptorSetLayout> setLayouts;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
#ifdef MK_OPTIMIZATION_GLOBAL_UNIFORM_BUFFERS
	setLayouts.push_back(*descriptor->getDynamicUniformBufferDescriptorSetLayout());
	setLayouts.push_back(*descriptor->getDynamicUniformBufferDescriptorSetLayout());
#else
	setLayouts.push_back(*descriptor->getGeometryPassVertexDynamicDescriptorSetLayout());
	setLayouts.push_back(*descriptor->getShadowPassVertexDynamicDescriptorSetLayout());
#endif
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

	auto depthStenctilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(true).setDepthWriteEnable(true).setDepthCompareOp(vk::CompareOp::eLessOrEqual);

	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(0);

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPDepthStencilState(&depthStenctilStateCreateInfo);
	pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

ShadowPipeline::ShadowPipeline(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor)
{
	this->context = context;

	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, buffers, descriptor), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(renderPass.get(), pipelineLayout.get(), context), pipelineDeleter);
}