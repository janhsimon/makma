#include "Descriptor.hpp"

vk::DescriptorPool *Descriptor::createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials)
{
	auto texturesPoolSize = vk::DescriptorPoolSize().setDescriptorCount(numMaterials * 2).setType(vk::DescriptorType::eCombinedImageSampler);
	std::vector<vk::DescriptorPoolSize> poolSizes = { texturesPoolSize };

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	auto uboPoolSize = vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBuffer);
	poolSizes.push_back(uboPoolSize);
#endif

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data()).setMaxSets(numMaterials);
	auto descriptorPool = context->getDevice()->createDescriptorPool(descriptorPoolCreateInfo);
	return new vk::DescriptorPool(descriptorPool);
}

vk::DescriptorSetLayout *Descriptor::createDescriptorSetLayout(const std::shared_ptr<Context> context)
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
vk::DescriptorSetLayout *Descriptor::createUBODescriptorSetLayout(const std::shared_ptr<Context> context)
{
	auto uboLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eUniformBuffer);
	uboLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eVertex);

	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&uboLayoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet *Descriptor::createUBODescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptorPool).setDescriptorSetCount(1).setPSetLayouts(descriptorSetLayout);
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto uboDescriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffers->getUniformBuffer()).setRange(sizeof(UniformBufferObject));
	auto diffuseSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eUniformBuffer);
	diffuseSamplerWriteDescriptorSet.setDescriptorCount(1).setPBufferInfo(&uboDescriptorBufferInfo);

	context->getDevice()->updateDescriptorSets(1, &diffuseSamplerWriteDescriptorSet, 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}
#endif

Descriptor::Descriptor(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, uint32_t numMaterials)
{
	this->context = context;
	
	descriptorPool = std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)>(createDescriptorPool(context, numMaterials), descriptorPoolDeleter);
	descriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createDescriptorSetLayout(context), descriptorSetLayoutDeleter);

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	uboDescriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createUBODescriptorSetLayout(context), descriptorSetLayoutDeleter);
	uboDescriptorSet = std::unique_ptr<vk::DescriptorSet>(createUBODescriptorSet(context, buffers, descriptorPool.get(), uboDescriptorSetLayout.get()));
#endif
}