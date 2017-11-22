#include "Descriptor.hpp"

vk::DescriptorPool *Descriptor::createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials)
{
	auto poolSize = vk::DescriptorPoolSize().setDescriptorCount(numMaterials * 2).setType(vk::DescriptorType::eCombinedImageSampler);
	
#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	poolSize.setDescriptorCount(numMaterials * 3);
#endif

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setPoolSizeCount(1).setPPoolSizes(&poolSize).setMaxSets(numMaterials);
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

Descriptor::Descriptor(const std::shared_ptr<Context> context, uint32_t numMaterials)
{
	this->context = context;
	
	descriptorPool = std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)>(createDescriptorPool(context, numMaterials), descriptorPoolDeleter);
	descriptorSetLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)>(createDescriptorSetLayout(context), descriptorSetLayoutDeleter);
}