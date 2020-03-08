#pragma once

#include "DescriptorPool.hpp"
#include "../Context.hpp"

class Descriptor
{
private:
	std::shared_ptr<Context> context;
	std::shared_ptr<DescriptorPool> descriptorPool;

	static vk::DescriptorSetLayout *createLayout(const std::shared_ptr<Context> context, vk::DescriptorType type, vk::ShaderStageFlags shaderStageFlags);
	std::function<void(vk::DescriptorSetLayout*)> layoutDeleter = [this](vk::DescriptorSetLayout *layout) { if (context->getDevice()) context->getDevice()->destroyDescriptorSetLayout(*layout); };
	std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)> layout;

	static vk::DescriptorSet *createSet(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const vk::DescriptorSetLayout *layout, const vk::Buffer *buffer, vk::DeviceSize range, vk::DescriptorType type);
	std::unique_ptr<vk::DescriptorSet> set;

public:
	Descriptor(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, vk::DescriptorType type, vk::ShaderStageFlags shaderStageFlags, const vk::Buffer *buffer, vk::DeviceSize range);

	vk::DescriptorSetLayout *getLayout() const { return layout.get(); }
	vk::DescriptorSet *getSet() const { return set.get(); }
};