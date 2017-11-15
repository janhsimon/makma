#include "Buffers.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"

vk::DescriptorPool *Pipeline::createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numDiffuseTextures, uint32_t numNormalTextures)
{
	auto diffuseSamplerPoolSize = vk::DescriptorPoolSize().setDescriptorCount(numDiffuseTextures).setType(vk::DescriptorType::eCombinedImageSampler);
	auto normalSamplerPoolSize = vk::DescriptorPoolSize().setDescriptorCount(numNormalTextures).setType(vk::DescriptorType::eCombinedImageSampler);
	std::vector<vk::DescriptorPoolSize> poolSizes = { diffuseSamplerPoolSize, normalSamplerPoolSize };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	auto uboPoolSize = vk::DescriptorPoolSize().setDescriptorCount(numTextures).setType(vk::DescriptorType::eUniformBuffer);
	poolSizes.push_back(uboPoolSize);
#endif

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data());
	descriptorPoolCreateInfo.setMaxSets(numDiffuseTextures);
	auto descriptorPool = context->getDevice()->createDescriptorPool(descriptorPoolCreateInfo);
	return new vk::DescriptorPool(descriptorPool);
}

std::vector<vk::DescriptorSetLayout> *Pipeline::createDescriptorSetLayouts(const std::shared_ptr<Context> context, uint32_t numDiffuseTextures, uint32_t numNormalTextures)
{
	auto descriptorSetLayouts = new std::vector<vk::DescriptorSetLayout>();

	for (uint32_t i = 0; i < numDiffuseTextures; ++i)
	{
		auto diffuseSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		diffuseSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

		auto normalSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		normalSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);
		
		std::vector<vk::DescriptorSetLayoutBinding> bindings = { diffuseSamplerLayoutBinding, normalSamplerLayoutBinding };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
		auto uboLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer);
		uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);
		bindings.push_back(uboLayoutBinding);
#endif

		auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(static_cast<uint32_t>(bindings.size())).setPBindings(bindings.data());
		descriptorSetLayouts->push_back(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
	}

	return descriptorSetLayouts;
}

std::vector<vk::DescriptorSet> *Pipeline::createDescriptorSets(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<std::vector<Texture*>> diffuseTextures, const std::shared_ptr<std::vector<Texture*>> normalTextures, const std::vector<vk::DescriptorSetLayout> *descriptorSetLayouts, const vk::DescriptorPool *descriptorPool)
{
	auto descriptorSetsAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(static_cast<uint32_t>(diffuseTextures->size()));
	descriptorSetsAllocateInfo.setPSetLayouts(descriptorSetLayouts->data());
	auto descriptorSets = context->getDevice()->allocateDescriptorSets(descriptorSetsAllocateInfo);

	for (auto i = 0; i < diffuseTextures->size(); ++i)
	{
		auto diffuseTexture = diffuseTextures->at(i);
		auto diffuseDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*diffuseTexture->getImageView()).setSampler(*diffuseTexture->getSampler());
		auto diffuseSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSets.at(i)).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		diffuseSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&diffuseDescriptorImageInfo);

		auto normalTexture = normalTextures->at(i);
		auto normalDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*normalTexture->getImageView()).setSampler(*normalTexture->getSampler());
		auto normalSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(1).setDstSet(descriptorSets.at(i)).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
		normalSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&normalDescriptorImageInfo);
		
		std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { diffuseSamplerWriteDescriptorSet, normalSamplerWriteDescriptorSet };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
		auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getUniformBuffer()).setRange(sizeof(UniformBufferObject));
		auto uboWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(2).setDstSet(descriptorSets.at(i)).setDescriptorType(vk::DescriptorType::eUniformBuffer);
		uboWriteDescriptorSet.setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
		writeDescriptorSets.push_back(uboWriteDescriptorSet);
#endif

		context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}

	return new std::vector<vk::DescriptorSet>(descriptorSets);
}

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

vk::PipelineLayout *Pipeline::createPipelineLayout(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::vector<vk::DescriptorSetLayout> *descriptorSetLayouts)
{
	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(1).setPSetLayouts(&descriptorSetLayouts->at(0));

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
	auto color = vk::VertexInputAttributeDescription().setLocation(1).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, color));
	auto texCoord = vk::VertexInputAttributeDescription().setLocation(2).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(Vertex, texCoord));
	std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position, color, texCoord };
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

Pipeline::Pipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<std::vector<Texture*>> diffuseTextures, const std::shared_ptr<std::vector<Texture*>> normalTextures)
{
	this->context = context;

	descriptorPool = std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)>(createDescriptorPool(context, static_cast<uint32_t>(diffuseTextures->size()), static_cast<uint32_t>(normalTextures->size())), descriptorPoolDeleter);
	descriptorSetLayouts = std::unique_ptr<std::vector<vk::DescriptorSetLayout>, decltype(descriptorSetLayoutsDeleter)>(createDescriptorSetLayouts(context, static_cast<uint32_t>(diffuseTextures->size()), static_cast<uint32_t>(normalTextures->size())), descriptorSetLayoutsDeleter);
	descriptorSets = std::unique_ptr<std::vector<vk::DescriptorSet>>(createDescriptorSets(context, buffers, diffuseTextures, normalTextures, descriptorSetLayouts.get(), descriptorPool.get()));
	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, buffers, descriptorSetLayouts.get()), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass.get(), pipelineLayout.get(), context), pipelineDeleter);
}