#include "Buffers.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"

vk::RenderPass *Pipeline::createRenderPass(const std::shared_ptr<Context> context)
{
	auto colorAttachmentDescription = vk::AttachmentDescription().setFormat(vk::Format::eB8G8R8A8Unorm).setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAttachmentDescription.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	auto colorAttachmentReference = vk::AttachmentReference().setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachmentCount(1).setPColorAttachments(&colorAttachmentReference);

	auto subpassDependency = vk::SubpassDependency().setSrcSubpass(VK_SUBPASS_EXTERNAL).setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	auto renderPassCreateInfo = vk::RenderPassCreateInfo().setAttachmentCount(1).setPAttachments(&colorAttachmentDescription);
	renderPassCreateInfo.setSubpassCount(1).setPSubpasses(&subpassDescription).setDependencyCount(1).setPDependencies(&subpassDependency);
	auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
	return new vk::RenderPass(renderPass);
}

vk::PipelineLayout *Pipeline::createPipelineLayout(const std::shared_ptr<Context> context)
{
	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo();
	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *Pipeline::createPipeline(const Window *window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, std::shared_ptr<Context> context)
{
	Shader vertexShader("Shaders\\vert.spv", context);
	Shader fragmentShader("Shaders\\frag.spv", context);

	auto vertexShaderInfo = vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eVertex).setModule(*vertexShader.getShaderModule()).setPName("main");
	auto fragmentShaderInfo = vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eFragment).setModule(*fragmentShader.getShaderModule()).setPName("main");
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShaderInfo, fragmentShaderInfo };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto vertexInputAttributeDescriptionPosition = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(Vertex, position));
	auto vertexInputAttributeDescriptionColor = vk::VertexInputAttributeDescription().setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, color));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { vertexInputAttributeDescriptionPosition, vertexInputAttributeDescriptionColor };
	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vertexInputBindingDescription);
	vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(static_cast<uint32_t>(vertexInputAttributeDescriptions.size()));
	vertexInputStateCreateInfo.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions.data());
	
	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto viewport = vk::Viewport().setWidth(window->getWidth()).setHeight(window->getHeight()).setMaxDepth(1.0f);
	auto scissor = vk::Rect2D().setExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);
	
	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eBack).setFrontFace(vk::FrontFace::eClockwise).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setSrcColorBlendFactor(vk::BlendFactor::eOne).setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
	colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(1).setPAttachments(&colorBlendAttachmentState);

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

Pipeline::Pipeline(const Window *window, const std::shared_ptr<Context> context)
{
	this->context = context;

	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass.get(), pipelineLayout.get(), context), pipelineDeleter);
}