#include "Descriptor.hpp"

vk::DescriptorPool *Descriptor::createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials, uint32_t numShadowMaps)
{
	// we need one descriptor set per texture (five textures per material), one per shadow map and one for each of the three textures in the geometry buffer
	std::vector<vk::DescriptorPoolSize> poolSizes = { vk::DescriptorPoolSize().setDescriptorCount(numMaterials * 5 + numShadowMaps + 3).setType(vk::DescriptorType::eCombinedImageSampler) };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
#ifdef MK_OPTIMIZATION_GLOBAL_UNIFORM_BUFFERS
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBufferDynamic));
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBuffer));
#else
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(4).setType(vk::DescriptorType::eUniformBufferDynamic));
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(2).setType(vk::DescriptorType::eUniformBuffer));
#endif
#endif

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data());

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
#ifdef MK_OPTIMIZATION_GLOBAL_UNIFORM_BUFFERS
	descriptorPoolCreateInfo.setMaxSets(numMaterials + 3 + numShadowMaps);
#else
	descriptorPoolCreateInfo.setMaxSets(numMaterials + 7 + numShadowMaps);
#endif
#endif

	auto descriptorPool = context->getDevice()->createDescriptorPool(descriptorPoolCreateInfo);
	return new vk::DescriptorPool(descriptorPool);
}

vk::DescriptorSetLayout *Descriptor::createMaterialDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto diffuseSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	diffuseSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto normalSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto occlusionSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	occlusionSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto metallicSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(3).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	metallicSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto roughnessSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(4).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	roughnessSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings = { diffuseSamplerLayoutBinding, normalSamplerLayoutBinding, occlusionSamplerLayoutBinding, metallicSamplerLayoutBinding, roughnessSamplerLayoutBinding };
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(static_cast<uint32_t>(bindings.size())).setPBindings(bindings.data());
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSetLayout *Descriptor::createShadowMapMaterialDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto shadowMapSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	shadowMapSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&shadowMapSamplerLayoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSetLayout *Descriptor::createGeometryBufferDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	// world-space position
	auto positionSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	positionSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	// albedo
	auto albedoSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	albedoSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	// world-space normal
	auto normalSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings = { positionSamplerLayoutBinding, albedoSamplerLayoutBinding, normalSamplerLayoutBinding };
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(static_cast<uint32_t>(bindings.size())).setPBindings(bindings.data());
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
#ifdef MK_OPTIMIZATION_GLOBAL_UNIFORM_BUFFERS
vk::DescriptorSetLayout *Descriptor::createUniformBufferDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createUniformBufferDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getUniformBuffer()).setRange(sizeof(UniformBufferData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createDynamicUniformBufferDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createDynamicUniformBufferDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getDynamicUniformBuffer()).setRange(sizeof(DynamicUniformBufferData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}
#else
vk::DescriptorSetLayout *Descriptor::createShadowPassVertexDynamicDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eVertex);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createShadowPassVertexDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getShadowPassVertexDynamicUniformBuffer()).setRange(sizeof(ShadowPassVertexDynamicData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createGeometryPassVertexDynamicDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eVertex);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createGeometryPassVertexDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getGeometryPassVertexDynamicUniformBuffer()).setRange(sizeof(GeometryPassVertexDynamicData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createGeometryPassVertexDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eVertex);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createGeometryPassVertexDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getGeometryPassVertexUniformBuffer()).setRange(sizeof(GeometryPassVertexData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createLightingPassVertexDynamicDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eVertex);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createLightingPassVertexDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getLightingPassVertexDynamicUniformBuffer()).setRange(sizeof(LightingPassVertexDynamicData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createLightingPassFragmentDynamicDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eFragment);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createLightingPassFragmentDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getLightingPassFragmentDynamicUniformBuffer()).setRange(sizeof(LightingPassFragmentDynamicData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createLightingPassFragmentDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eFragment);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createLightingPassFragmentDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getLightingPassFragmentUniformBuffer()).setRange(sizeof(LightingPassFragmentData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}
#endif
#endif

Descriptor::Descriptor(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, uint32_t numMaterials, uint32_t numShadowMaps)
{
	this->context = context;
	
	descriptorPool = std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)>(createDescriptorPool(context, numMaterials, numShadowMaps), descriptorPoolDeleter);

	materialDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createMaterialDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	shadowMapMaterialDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createShadowMapMaterialDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	geometryBufferDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createGeometryBufferDescriptorSetLayout(context), descriptorSetLayoutDeleter);

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
#ifdef MK_OPTIMIZATION_GLOBAL_UNIFORM_BUFFERS
	uniformBufferDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createUniformBufferDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	uniformBufferDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createUniformBufferDescriptorSet(context, buffers, descriptorPool.get(), uniformBufferDescriptorSetLayout.get()));

	dynamicUniformBufferDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createDynamicUniformBufferDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	dynamicUniformBufferDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createDynamicUniformBufferDescriptorSet(context, buffers, descriptorPool.get(), dynamicUniformBufferDescriptorSetLayout.get()));
#else
	shadowPassVertexDynamicDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createShadowPassVertexDynamicDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	shadowPassVertexDynamicDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createShadowPassVertexDynamicDescriptorSet(context, buffers, descriptorPool.get(), shadowPassVertexDynamicDescriptorSetLayout.get()));
	
	geometryPassVertexDynamicDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createGeometryPassVertexDynamicDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	geometryPassVertexDynamicDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createGeometryPassVertexDynamicDescriptorSet(context, buffers, descriptorPool.get(), geometryPassVertexDynamicDescriptorSetLayout.get()));
	
	geometryPassVertexDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createGeometryPassVertexDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	geometryPassVertexDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createGeometryPassVertexDescriptorSet(context, buffers, descriptorPool.get(), geometryPassVertexDescriptorSetLayout.get()));

	lightingPassVertexDynamicDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createLightingPassVertexDynamicDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	lightingPassVertexDynamicDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createLightingPassVertexDynamicDescriptorSet(context, buffers, descriptorPool.get(), lightingPassVertexDynamicDescriptorSetLayout.get()));

	lightingPassFragmentDynamicDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createLightingPassFragmentDynamicDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	lightingPassFragmentDynamicDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createLightingPassFragmentDynamicDescriptorSet(context, buffers, descriptorPool.get(), lightingPassFragmentDynamicDescriptorSetLayout.get()));

	lightingPassFragmentDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createLightingPassFragmentDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	lightingPassFragmentDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createLightingPassFragmentDescriptorSet(context, buffers, descriptorPool.get(), lightingPassFragmentDescriptorSetLayout.get()));
#endif
#endif
}