#include "DescriptorPool.hpp"
#include "../Settings.hpp"

vk::DescriptorPool *DescriptorPool::createPool(const std::shared_ptr<Context> context, uint32_t numMaterials, uint32_t numShadowMaps)
{
	// we need one descriptor set per texture (five textures per material), one per shadow map, one for the ui font,
	// one for each of the two textures in the geometry buffer and one for each for each of the two textures in the lighting buffer
	// TODO: why is MK_OPTIMIZATION_SHADOW_MAP_MAX_CASCADE_COUNT needed?
	std::vector<vk::DescriptorPoolSize> poolSizes = { vk::DescriptorPoolSize().setDescriptorCount(numMaterials * 5 + numShadowMaps + 1 + 2 + 2 + MK_OPTIMIZATION_SHADOW_MAP_MAX_CASCADE_COUNT).setType(vk::DescriptorType::eCombinedImageSampler) };

	uint32_t maxSets = 0;
	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		maxSets = numMaterials + 8 + MK_OPTIMIZATION_SHADOW_MAP_MAX_CASCADE_COUNT + numShadowMaps;
		poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(5).setType(vk::DescriptorType::eUniformBufferDynamic));
		poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBuffer));
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		maxSets = numMaterials + 8 + MK_OPTIMIZATION_SHADOW_MAP_MAX_CASCADE_COUNT + numShadowMaps; // TODO: is MK_OPTIMIZATION_SHADOW_MAP_MAX_CASCADE_COUNT even necessary HERE?
		poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(5).setType(vk::DescriptorType::eUniformBufferDynamic));
		poolSizes.push_back(vk::DescriptorPoolSize().setDescriptorCount(1).setType(vk::DescriptorType::eUniformBuffer));
	}

	auto descriptorPoolCreateInfo = vk::DescriptorPoolCreateInfo().setMaxSets(maxSets).setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
	descriptorPoolCreateInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size())).setPPoolSizes(poolSizes.data());
	
	auto descriptorPool = context->getDevice()->createDescriptorPool(descriptorPoolCreateInfo);
	return new vk::DescriptorPool(descriptorPool);
}

vk::DescriptorSetLayout *DescriptorPool::createMaterialLayout(const std::shared_ptr<Context> context)
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

vk::DescriptorSetLayout *DescriptorPool::createShadowMapLayout(const std::shared_ptr<Context> context)
{
	auto shadowMapSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	shadowMapSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&shadowMapSamplerLayoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSetLayout *DescriptorPool::createGeometryBufferLayout(const std::shared_ptr<Context> context)
{
	auto albedoSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	albedoSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto normalSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto depthSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(2).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	depthSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings = { albedoSamplerLayoutBinding, normalSamplerLayoutBinding, depthSamplerLayoutBinding };
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(static_cast<uint32_t>(bindings.size())).setPBindings(bindings.data());
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSetLayout *DescriptorPool::createLightingBufferLayout(const std::shared_ptr<Context> context)
{
	auto fullscaleSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	fullscaleSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto halfscaleSamplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(1).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	halfscaleSamplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings = { fullscaleSamplerLayoutBinding, halfscaleSamplerLayoutBinding };
	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(static_cast<uint32_t>(bindings.size())).setPBindings(bindings.data());
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSetLayout *DescriptorPool::createFontLayout(const std::shared_ptr<Context> context)
{
	auto samplerLayoutBinding = vk::DescriptorSetLayoutBinding().setBinding(0).setDescriptorCount(1).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	samplerLayoutBinding.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&samplerLayoutBinding);
	return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

DescriptorPool::DescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials, uint32_t numShadowMaps)
{
	this->context = context;
	pool = std::unique_ptr<vk::DescriptorPool, decltype(poolDeleter)>(createPool(context, numMaterials, numShadowMaps), poolDeleter);

	materialLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)>(createMaterialLayout(context), layoutDeleter);
	shadowMapLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)>(createShadowMapLayout(context), layoutDeleter);
	geometryBufferLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)>(createGeometryBufferLayout(context), layoutDeleter);
	lightingBufferLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)>(createLightingBufferLayout(context), layoutDeleter);
	fontLayout = std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)>(createFontLayout(context), layoutDeleter);
}