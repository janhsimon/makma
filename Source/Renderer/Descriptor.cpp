#include "Descriptor.hpp"

vk::DescriptorPool *Descriptor::createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials)
{
	// we need one descriptor set per texture (five textures per material) and one for each of the three textures in the geometry buffer
	std::vector<vk::DescriptorPoolSize> poolSizes = { vk::DescriptorPoolSize().setDescriptorCount(numMaterials * 5 + 3).setType(vk::DescriptorType::eCombinedImageSampler) };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(2).setType(vk::DescriptorType::eUniformBufferDynamic)); // world matrix and light data
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(2).setType(vk::DescriptorType::eUniformBuffer)); // view-projection matrix and eye position
#endif

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data()).setMaxSets(numMaterials + 5);
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
vk::DescriptorSetLayout *Descriptor::createWorldMatrixDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eVertex);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createWorldMatrixDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getWorldUniformBuffer()).setRange(sizeof(WorldData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createLightDataDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setStageFlags(vk::ShaderStageFlagBits::eFragment);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createLightDataDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getLightUniformBuffer()).setRange(sizeof(LightData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBufferDynamic).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createViewProjectionMatrixDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eVertex);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createViewProjectionMatrixDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getViewProjectionUniformBuffer()).setRange(sizeof(ViewProjectionData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

vk::DescriptorSetLayout *Descriptor::createEyePositionDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto layoutBinding = vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer).setStageFlags(vk::ShaderStageFlagBits::eFragment);
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createEyePositionDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getEyePositionUniformBuffer()).setRange(sizeof(EyePositionData));
	auto writeDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(1).setPBufferInfo(&descriptorBufferInfo);
	context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}
#endif

Descriptor::Descriptor(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, uint32_t numMaterials)
{
	this->context = context;
	
	descriptorPool = std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)>(createDescriptorPool(context, numMaterials), descriptorPoolDeleter);

	materialDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createMaterialDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	geometryBufferDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createGeometryBufferDescriptorSetLayout(context), descriptorSetLayoutDeleter);

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	worldMatrixDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createWorldMatrixDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	worldMatrixDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createWorldMatrixDescriptorSet(context, buffers, descriptorPool.get(), worldMatrixDescriptorSetLayout.get()));

	lightDataDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createLightDataDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	lightDataDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createLightDataDescriptorSet(context, buffers, descriptorPool.get(), lightDataDescriptorSetLayout.get()));

	viewProjectionMatrixDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createViewProjectionMatrixDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	viewProjectionMatrixDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createViewProjectionMatrixDescriptorSet(context, buffers, descriptorPool.get(), viewProjectionMatrixDescriptorSetLayout.get()));

	eyePositionDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createEyePositionDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	eyePositionDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createEyePositionDescriptorSet(context, buffers, descriptorPool.get(), eyePositionDescriptorSetLayout.get()));
#endif
}