#include "LightingPipelines.hpp"
#include "../Settings.hpp"
#include "../Shader.hpp"
#include "../Buffers/VertexBuffer.hpp"

vk::PipelineLayout *LightingPipelines::createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts)
{
	auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo().setSetLayoutCount(static_cast<uint32_t>(setLayouts.size())).setPSetLayouts(setLayouts.data());
	auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
	return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline *LightingPipelines::createPipelineNoShadowMaps(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout)
{
	struct SpecializationData
	{
		//int shadowMapCascadeCount = Settings::shadowMapCascadeCount;
		//float shadowBias = Settings::shadowBias;
		//int shadowFilterRange = Settings::shadowFilterRange;
		float bloomThreshold = Settings::bloomThreshold;
		float volumetricIntensity = Settings::volumetricIntensity;
		int volumetricSteps = Settings::volumetricSteps;
		float volumetricScattering = Settings::volumetricScattering;
	}
	specializationData;

	std::vector<vk::SpecializationMapEntry> specializationConstants;
	//specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(0).setOffset(offsetof(SpecializationData, shadowMapCascadeCount)).setSize(sizeof(specializationData.shadowMapCascadeCount)));
	//specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(1).setOffset(offsetof(SpecializationData, shadowBias)).setSize(sizeof(specializationData.shadowBias)));
	//specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(2).setOffset(offsetof(SpecializationData, shadowFilterRange)).setSize(sizeof(specializationData.shadowFilterRange)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(0).setOffset(offsetof(SpecializationData, bloomThreshold)).setSize(sizeof(specializationData.bloomThreshold)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(1).setOffset(offsetof(SpecializationData, volumetricIntensity)).setSize(sizeof(specializationData.volumetricIntensity)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(2).setOffset(offsetof(SpecializationData, volumetricSteps)).setSize(sizeof(specializationData.volumetricSteps)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(3).setOffset(offsetof(SpecializationData, volumetricScattering)).setSize(sizeof(specializationData.volumetricScattering)));
	auto specializationInfo = vk::SpecializationInfo().setMapEntryCount(static_cast<uint32_t>(specializationConstants.size())).setPMapEntries(specializationConstants.data());
	specializationInfo.setDataSize(sizeof(specializationData)).setPData(&specializationData);

	Shader vertexShader(context, "Shaders/LightingPassNoShadowMaps.vert.spv", vk::ShaderStageFlagBits::eVertex);
	Shader fragmentShader(context, "Shaders/LightingPassNoShadowMaps.frag.spv", vk::ShaderStageFlagBits::eFragment);

	auto fragmentShaderStageCreateInfo = fragmentShader.getPipelineShaderStageCreateInfo().setPSpecializationInfo(&specializationInfo);
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShaderStageCreateInfo };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, position));
	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vertexInputBindingDescription);
	vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(1).setPVertexAttributeDescriptions(&position);

	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto viewport = vk::Viewport().setWidth(window->getWidth()).setHeight(window->getHeight()).setMaxDepth(1.0f);
	auto scissor = vk::Rect2D().setExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);

	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eFront).setFrontFace(vk::FrontFace::eCounterClockwise).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo();

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	colorBlendAttachmentState.setColorBlendOp(vk::BlendOp::eAdd).setSrcColorBlendFactor(vk::BlendFactor::eOne).setDstColorBlendFactor(vk::BlendFactor::eOne).setBlendEnable(true);
	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = { colorBlendAttachmentState, colorBlendAttachmentState };
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(static_cast<uint32_t>(colorBlendAttachmentStates.size())).setPAttachments(colorBlendAttachmentStates.data());

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

vk::Pipeline *LightingPipelines::createPipelineWithShadowMaps(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, const vk::RenderPass *renderPass, const vk::PipelineLayout *pipelineLayout)
{
	struct SpecializationData
	{
		int shadowMapCascadeCount = Settings::shadowMapCascadeCount;
		float shadowBias = Settings::shadowBias;
		int shadowFilterRange = Settings::shadowFilterRange;
		float bloomThreshold = Settings::bloomThreshold;
		float volumetricIntensity = Settings::volumetricIntensity;
		int volumetricSteps = Settings::volumetricSteps;
		float volumetricScattering = Settings::volumetricScattering;
	}
	specializationData;

	std::vector<vk::SpecializationMapEntry> specializationConstants;
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(0).setOffset(offsetof(SpecializationData, shadowMapCascadeCount)).setSize(sizeof(specializationData.shadowMapCascadeCount)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(1).setOffset(offsetof(SpecializationData, shadowBias)).setSize(sizeof(specializationData.shadowBias)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(2).setOffset(offsetof(SpecializationData, shadowFilterRange)).setSize(sizeof(specializationData.shadowFilterRange)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(3).setOffset(offsetof(SpecializationData, bloomThreshold)).setSize(sizeof(specializationData.bloomThreshold)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(4).setOffset(offsetof(SpecializationData, volumetricIntensity)).setSize(sizeof(specializationData.volumetricIntensity)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(5).setOffset(offsetof(SpecializationData, volumetricSteps)).setSize(sizeof(specializationData.volumetricSteps)));
	specializationConstants.push_back(vk::SpecializationMapEntry().setConstantID(6).setOffset(offsetof(SpecializationData, volumetricScattering)).setSize(sizeof(specializationData.volumetricScattering)));
	auto specializationInfo = vk::SpecializationInfo().setMapEntryCount(static_cast<uint32_t>(specializationConstants.size())).setPMapEntries(specializationConstants.data());
	specializationInfo.setDataSize(sizeof(specializationData)).setPData(&specializationData);

	Shader vertexShader(context, "Shaders/LightingPassWithShadowMaps.vert.spv", vk::ShaderStageFlagBits::eVertex);
	Shader fragmentShader(context, "Shaders/LightingPassWithShadowMaps.frag.spv", vk::ShaderStageFlagBits::eFragment);

	auto fragmentShaderStageCreateInfo = fragmentShader.getPipelineShaderStageCreateInfo().setPSpecializationInfo(&specializationInfo);
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = { vertexShader.getPipelineShaderStageCreateInfo(), fragmentShaderStageCreateInfo };

	auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
	auto position = vk::VertexInputAttributeDescription().setLocation(0).setFormat(vk::Format::eR32G32B32Sfloat).setOffset(offsetof(Vertex, position));
	auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(&vertexInputBindingDescription);
	vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(1).setPVertexAttributeDescriptions(&position);

	auto inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

	auto viewport = vk::Viewport().setWidth(window->getWidth()).setHeight(window->getHeight()).setMaxDepth(1.0f);
	auto scissor = vk::Rect2D().setExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
	auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(&scissor);

	auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eFront).setFrontFace(vk::FrontFace::eCounterClockwise).setLineWidth(1.0f);

	auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo();

	auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState().setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
	colorBlendAttachmentState.setColorBlendOp(vk::BlendOp::eAdd).setSrcColorBlendFactor(vk::BlendFactor::eOne).setDstColorBlendFactor(vk::BlendFactor::eOne).setBlendEnable(true);
	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = { colorBlendAttachmentState, colorBlendAttachmentState };
	auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(static_cast<uint32_t>(colorBlendAttachmentStates.size())).setPAttachments(colorBlendAttachmentStates.data());

	auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo().setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size())).setPStages(pipelineShaderStageCreateInfos.data());
	pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo).setPInputAssemblyState(&inputAssemblyStateCreateInfo).setPViewportState(&viewportStateCreateInfo);
	pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo).setPMultisampleState(&multisampleStateCreateInfo).setPColorBlendState(&colorBlendStateCreateInfo);
	pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
	auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
	return new vk::Pipeline(pipeline);
}

LightingPipelines::LightingPipelines(const std::shared_ptr<Window> window, const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayoutsNoShadowMaps, std::vector<vk::DescriptorSetLayout> setLayoutsWithShadowMaps,const vk::RenderPass *renderPass)
{
	this->context = context;

	pipelineLayoutNoShadowMaps = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, setLayoutsNoShadowMaps), pipelineLayoutDeleter);
	pipelineNoShadowMaps = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipelineNoShadowMaps(window, context, renderPass, pipelineLayoutNoShadowMaps.get()), pipelineDeleter);

	pipelineLayoutWithShadowMaps = std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, setLayoutsWithShadowMaps), pipelineLayoutDeleter);
	pipelineWithShadowMaps = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipelineWithShadowMaps(window, context, renderPass, pipelineLayoutWithShadowMaps.get()), pipelineDeleter);
}
