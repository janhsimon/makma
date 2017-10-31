#include "Buffers.hpp"
#include "Pipeline.hpp"
#include "Shader.hpp"

vk::DescriptorSetLayout *Pipeline::createDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto uboLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer);
	uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto samplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	samplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings = { uboLayoutBinding, samplerLayoutBinding };
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(static_cast<uint32_t>(bindings.size())).setPBindings(bindings.data());
	auto descriptorSetLayout = context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
	return new vk::DescriptorSetLayout(descriptorSetLayout);
}

vk::DescriptorPool *Pipeline::createDescriptorPool(const std::shared_ptr<Context> context)
{
	auto uboPoolSize = vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBuffer);
	auto samplerPoolSize = vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eCombinedImageSampler);

	std::vector<vk::DescriptorPoolSize> poolSizes = { uboPoolSize, samplerPoolSize };
	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data()).setMaxSets(1);
	auto descriptorPool = context->getDevice()->createDescriptorPool(descriptorPoolCreateInfo);
	return new vk::DescriptorPool(descriptorPool);
}

vk::DescriptorSet *Pipeline::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Texture> texture, const vk::DescriptorSetLayout *descriptorSetLayout, const vk::DescriptorPool* descriptorPool)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getUniformBuffer()).setRange(sizeof(UniformBufferObject));
	auto uboWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer);
	uboWriteDescriptorSet.setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);

	auto descriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*texture->getImageView()).setSampler(*texture->getSampler());
	auto samplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(1).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	samplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&descriptorImageInfo);

	std::vector<vk::WriteDescriptorSet> descriptorWrites = { uboWriteDescriptorSet, samplerWriteDescriptorSet };
	context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);

	return new vk::DescriptorSet(descriptorSet);
}

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

vk::PipelineLayout *Pipeline::createPipelineLayout(const std::shared_ptr<Context> context, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(1).setPSetLayouts(descriptorSetLayout);
	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *Pipeline::createPipeline(const std::shared_ptr<Window> window, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout, std::shared_ptr<Context> context)
{
	// TODO: these filenames should really be passed as parameters
	Shader vertexShader("Shaders\\Shader.vert.spv", context);
	Shader fragmentShader("Shaders\\Shader.frag.spv", context);

	// TODO: the shader info could be moved into the shader objects with a getter for the vector below
	auto vertexShaderInfo = vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eVertex).setModule(*vertexShader.getShaderModule()).setPName("main");
	auto fragmentShaderInfo = vk::PipelineShaderStageCreateInfo().setStage(vk::ShaderStageFlagBits::eFragment).setModule(*fragmentShader.getShaderModule()).setPName("main");
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShaderInfo, fragmentShaderInfo };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32Sfloat).setOffset(offsetof(Vertex, position));
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

Pipeline::Pipeline(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const std::shared_ptr<Texture> texture)
{
	this->context = context;

	descriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	descriptorPool = std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)>(createDescriptorPool(context), descriptorPoolDeleter);
	descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, buffers, texture, descriptorSetLayout.get(), descriptorPool.get()));
	renderPass = std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
	pipelineLayout = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, descriptorSetLayout.get()), pipelineLayoutDeleter);
	pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass.get(), pipelineLayout.get(), context), pipelineDeleter);
}