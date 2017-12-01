#include "Descriptor.hpp"

vk::DescriptorPool *Descriptor::createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials)
{
	// TODO: test if numMaterials * 2 is needed here and at the bottom of the function!!!
	std::vector<vk::DescriptorPoolSize> poolSizes = { vk::DescriptorPoolSize().setDescriptorCount(numMaterials).setType(vk::DescriptorType::eCombinedImageSampler) };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBufferDynamic));
	poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBuffer));
	
#endif

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data()).setMaxSets(numMaterials);
	auto descriptorPool = context->getDevice()->createDescriptorPool(descriptorPoolCreateInfo);
	return new vk::DescriptorPool(descriptorPool);
}

vk::DescriptorSetLayout *Descriptor::createMaterialDescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto diffuseSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	diffuseSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto normalSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings = { diffuseSamplerLayoutBinding, normalSamplerLayoutBinding };
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
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getDynamicUniformBuffer()).setRange(VK_WHOLE_SIZE); // sizeof(DynamicUniformBufferData); buffers->getDynamicUniformBufferSize() / 2);
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
	auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getUniformBuffer()).setRange(sizeof(UniformBufferData));
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

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	worldMatrixDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createWorldMatrixDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	worldMatrixDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createWorldMatrixDescriptorSet(context, buffers, descriptorPool.get(), worldMatrixDescriptorSetLayout.get()));

	viewProjectionMatrixDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createViewProjectionMatrixDescriptorSetLayout(context), descriptorSetLayoutDeleter);
	viewProjectionMatrixDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createViewProjectionMatrixDescriptorSet(context, buffers, descriptorPool.get(), viewProjectionMatrixDescriptorSetLayout.get()));
#endif
}