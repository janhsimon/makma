#pragma once

#include "Context.hpp"

class Descriptor
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorPool *createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials);
	std::function<void(vk::DescriptorPool*)> descriptorPoolDeleter = [this](vk::DescriptorPool *descriptorPool) { if (context->getDevice()) context->getDevice()->destroyDescriptorPool(*descriptorPool); };
	std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)> descriptorPool;

	static vk::DescriptorSetLayout *createDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::function<void(vk::DescriptorSetLayout*)> descriptorSetLayoutDeleter = [this](vk::DescriptorSetLayout *descriptorSetLayout) { if (context->getDevice()) context->getDevice()->destroyDescriptorSetLayout(*descriptorSetLayout); };
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> descriptorSetLayout;

public:
	Descriptor(const std::shared_ptr<Context> context, uint32_t numMaterials);

	vk::DescriptorPool *getDescriptorPool() const { return descriptorPool.get(); }
	vk::DescriptorSetLayout *getDescriptorSetLayout() const { return descriptorSetLayout.get(); }
};