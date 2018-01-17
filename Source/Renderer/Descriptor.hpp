#pragma once

#include "Buffers.hpp"
#include "Context.hpp"

class Descriptor
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorPool *createDescriptorPool(const std::shared_ptr<Context> context, uint32_t numTextures, uint32_t numShadowMaps);
	std::function<void(vk::DescriptorPool*)> descriptorPoolDeleter = [this](vk::DescriptorPool *descriptorPool) { if (context->getDevice()) context->getDevice()->destroyDescriptorPool(*descriptorPool); };
	std::unique_ptr<vk::DescriptorPool, decltype(descriptorPoolDeleter)> descriptorPool;

	std::function<void(vk::DescriptorSetLayout*)> descriptorSetLayoutDeleter = [this](vk::DescriptorSetLayout *descriptorSetLayout) { if (context->getDevice()) context->getDevice()->destroyDescriptorSetLayout(*descriptorSetLayout); };

	static vk::DescriptorSetLayout *createMaterialDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> materialDescriptorSetLayout;

	static vk::DescriptorSetLayout *createShadowMapMaterialDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> shadowMapMaterialDescriptorSetLayout;

	static vk::DescriptorSetLayout *createGeometryBufferDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> geometryBufferDescriptorSetLayout;

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

	static vk::DescriptorSetLayout *createUniformBufferDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> uniformBufferDescriptorSetLayout;

	static vk::DescriptorSet *createUniformBufferDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> uniformBufferDescriptorSet;

	static vk::DescriptorSetLayout *createDynamicUniformBufferDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> dynamicUniformBufferDescriptorSetLayout;

	static vk::DescriptorSet *createDynamicUniformBufferDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> dynamicUniformBufferDescriptorSet;

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

	static vk::DescriptorSetLayout *createShadowPassDynamicDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> shadowPassDynamicDescriptorSetLayout;

	static vk::DescriptorSet *createShadowPassDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> shadowPassDynamicDescriptorSet;

	static vk::DescriptorSetLayout *createGeometryPassVertexDynamicDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> geometryPassVertexDynamicDescriptorSetLayout;
	
	static vk::DescriptorSet *createGeometryPassVertexDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> geometryPassVertexDynamicDescriptorSet;
	
	static vk::DescriptorSetLayout *createGeometryPassVertexDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> geometryPassVertexDescriptorSetLayout;

	static vk::DescriptorSet *createGeometryPassVertexDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> geometryPassVertexDescriptorSet;

	static vk::DescriptorSetLayout *createLightingPassVertexDynamicDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> lightingPassVertexDynamicDescriptorSetLayout;

	static vk::DescriptorSet *createLightingPassVertexDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> lightingPassVertexDynamicDescriptorSet;

	static vk::DescriptorSetLayout *createLightingPassVertexDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> lightingPassVertexDescriptorSetLayout;

	static vk::DescriptorSet *createLightingPassVertexDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> lightingPassVertexDescriptorSet;

	static vk::DescriptorSetLayout *createLightingPassFragmentDynamicDescriptorSetLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(descriptorSetLayoutDeleter)> lightingPassFragmentDynamicDescriptorSetLayout;

	static vk::DescriptorSet *createLightingPassFragmentDynamicDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, const vk::DescriptorPool *descriptorPool, const vk::DescriptorSetLayout *descriptorSetLayout);
	std::unique_ptr<vk::DescriptorSet> lightingPassFragmentDynamicDescriptorSet;

#endif

public:
	Descriptor(const std::shared_ptr<Context> context, const std::shared_ptr<Buffers> buffers, uint32_t numMaterials, uint32_t numShadowMaps);

	vk::DescriptorPool *getDescriptorPool() const { return descriptorPool.get(); }
	
	vk::DescriptorSetLayout *getMaterialDescriptorSetLayout() const { return materialDescriptorSetLayout.get(); }
	vk::DescriptorSetLayout *getShadowMapMaterialDescriptorSetLayout() const { return shadowMapMaterialDescriptorSetLayout.get(); }
	vk::DescriptorSetLayout *getGeometryBufferDescriptorSetLayout() const { return geometryBufferDescriptorSetLayout.get(); }

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

	vk::DescriptorSetLayout *getUniformBufferDescriptorSetLayout() const { return uniformBufferDescriptorSetLayout.get(); }
	vk::DescriptorSet *getUniformBufferDescriptorSet() const { return uniformBufferDescriptorSet.get(); }

	vk::DescriptorSetLayout *getDynamicUniformBufferDescriptorSetLayout() const { return dynamicUniformBufferDescriptorSetLayout.get(); }
	vk::DescriptorSet *getDynamicUniformBufferDescriptorSet() const { return dynamicUniformBufferDescriptorSet.get(); }

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

	vk::DescriptorSetLayout *getShadowPassDynamicDescriptorSetLayout() const { return shadowPassDynamicDescriptorSetLayout.get(); }
	vk::DescriptorSet *getShadowPassDynamicDescriptorSet() const { return shadowPassDynamicDescriptorSet.get(); }

	vk::DescriptorSetLayout *getGeometryPassVertexDynamicDescriptorSetLayout() const { return geometryPassVertexDynamicDescriptorSetLayout.get(); }
	vk::DescriptorSet *getGeometryPassVertexDynamicDescriptorSet() const { return geometryPassVertexDynamicDescriptorSet.get(); }

	vk::DescriptorSetLayout *getGeometryPassVertexDescriptorSetLayout() const { return geometryPassVertexDescriptorSetLayout.get(); }
	vk::DescriptorSet *getGeometryPassVertexDescriptorSet() const { return geometryPassVertexDescriptorSet.get(); }

	vk::DescriptorSetLayout *getLightingPassVertexDynamicDescriptorSetLayout() const { return lightingPassVertexDynamicDescriptorSetLayout.get(); }
	vk::DescriptorSet *getLightingPassVertexDynamicDescriptorSet() const { return lightingPassVertexDynamicDescriptorSet.get(); }

	vk::DescriptorSetLayout *getLightingPassVertexDescriptorSetLayout() const { return lightingPassVertexDescriptorSetLayout.get(); }
	vk::DescriptorSet *getLightingPassVertexDescriptorSet() const { return lightingPassVertexDescriptorSet.get(); }

	vk::DescriptorSetLayout *getLightingPassFragmentDynamicDescriptorSetLayout() const { return lightingPassFragmentDynamicDescriptorSetLayout.get(); }
	vk::DescriptorSet *getLightingPassFragmentDynamicDescriptorSet() const { return lightingPassFragmentDynamicDescriptorSet.get(); }

#endif
};