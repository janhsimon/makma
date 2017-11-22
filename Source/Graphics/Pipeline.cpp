#include "Buffers.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"

vk::RenderPass *Pipeline::createRenderPass(const std::shared_ptr<Context> context)
{
	auto colorAttachmentDescription = vk::AttachmentDescription().setFormat(vk::Format::eB8G8R8A8Unorm).setLoadOp(vk::AttachmentLoadOp::eClear);
	colorAttachmentDescription.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare).setFinalLayout(vk::ImageLayout::ePresentSrcKHR);
	auto colorAttachmentReference = vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto depthAttachmentDescription = vk::AttachmentDescription().setFormat(vk::Format::eD32Sfloat).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eDontCare);
	depthAttachmentDescription.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare).setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
	depthAttachmentDescription.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	auto depthAttachmentReference = vk::AttachmentReference().setAttachment(1).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpassDescription = vk::SubpassDescription().setPipelineBindPoint(vk::PipelineBindPoint::eGraphics).setColorAttachmentCount(1).setPColorAttachments(&colorAttachmentReference);
	subpassDescription.setPDepthStencilAttachment(&depthAttachmentReference);

	auto subpassDependency = vk::SubpassDependency().setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	subpassDependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput).setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::vector<vk::AttachmentDescription> attachmentDescriptions = { colorAttachmentDescription, depthAttachmentDescription };
	auto renderPassCreateInfo = vk::RenderPassCreateInfo().setAttachmentCount(static_cast<uint32_t>(attachmentDescriptions.size())).setPAttachments(attachmentDescriptions.data());
	renderPassCreateInfo.setSubpassCount(1).setPSubpasses(&subpassDescription).setDependencyCount(1).setPDependencies(&subpassDependency);
	auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
	return new vk::RenderPass(renderPass);
}

vk::PipelineLayout *Pipeline::createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor)
{
	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(1).setPSetLayouts(descriptor->getDescriptorSetLayout());

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	auto pushConstantRange = vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(*buffers->getPushConstants()));
	pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
	pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
#endif

	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *Pipeline::createPipeline(const std::shared_ptr<Window> window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, std::shared_ptr<Context> context)
{
#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	Shader vertexShader("Shaders\\PushConstants.vert.spv", vk::ShaderStageFlagBits::eVertex, context);
#else
	Shader vertexShader("Shaders\\Shader.vert.spv", vk::ShaderStageFlagBits::eVertex, context);
#endif

	Shader fragmentShader("Shaders\\Shader.frag.spv", vk::ShaderStageFlagBits::eFragment, context);

	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShader.getPipelineShaderStageCreateInfo() };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, position));
	auto texCoord = vk::VertexInputAttributeDescription().setLocation(1).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(Vertex, texCoord));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position, texCoord };
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
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(1).setPAttachments(&colorBlendAttachmentState);

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPDepthStencilState(&depthStenctilStateCreateInfo);
	pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

Pipeline::Pipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Descriptor> descriptor)
{
	this->context = context;

	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, buffers, descriptor), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass.get(), pipelineLayout.get(), context), pipelineDeleter);
}