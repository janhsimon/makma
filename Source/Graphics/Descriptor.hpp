#pragma once

#include "Buffers.hpp"
#include "Context.hpp"

class Descriptor
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorPool *createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials);
	std::function<void(vk::DescriptorPool*)> descriptorPoolDeleter = [this](vk::DescriptorPool *descriptorPool) { if (context->getDevice()) context->getDevice()->destroyDescriptorPool(*descriptorPool); };
	std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)> descriptorPool;

	std::function<void(vk::DescriptorSetLayout*)> descriptorSetLayoutDeleter = [this](vk::DescriptorSetLayout *descriptorSetLayout) { if (context->getDevice()) context->getDevice()->destroyDescriptorSetLayout(*descriptorSetLayout); };

	static vk::DescriptorSetLayout *createMaterialDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> materialDescriptorSetLayout;

	static vk::DescriptorSetLayout *createGeometryBufferDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> geometryBufferDescriptorSetLayout;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	static vk::DescriptorSetLayout *createWorldMatrixDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> worldMatrixDescriptorSetLayout;
	
	static vk::DescriptorSet *createWorldMatrixDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> worldMatrixDescriptorSet;
	
	static vk::DescriptorSetLayout *createViewProjectionMatrixDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> viewProjectionMatrixDescriptorSetLayout;

	static vk::DescriptorSet *createViewProjectionMatrixDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> viewProjectionMatrixDescriptorSet;
	
#endif

public:
	Descriptor(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, uint32_t numMaterials);

	vk::DescriptorPool *getDescriptorPool() const { return descriptorPool.get(); }
	
	vk::DescriptorSetLayout *getMaterialDescriptorSetLayout() const { return materialDescriptorSetLayout.get(); }
	vk::DescriptorSetLayout *getGeometryBufferDescriptorSetLayout() const { return geometryBufferDescriptorSetLayout.get(); }

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	vk::DescriptorSetLayout *getWorldMatrixDescriptorSetLayout() const { return worldMatrixDescriptorSetLayout.get(); }
	vk::DescriptorSet *getWorldMatrixDescriptorSet() const { return worldMatrixDescriptorSet.get(); }

	vk::DescriptorSetLayout *getViewProjectionMatrixDescriptorSetLayout() const { return viewProjectionMatrixDescriptorSetLayout.get(); }
	vk::DescriptorSet *getViewProjectionMatrixDescriptorSet() const { return viewProjectionMatrixDescriptorSet.get(); }
#endif
};