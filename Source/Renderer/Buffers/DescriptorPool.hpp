#pragma once

#include "../Context.hpp"

class DescriptorPool
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorPool *createPool(const std::shared_ptr<Context> context, uint32_t numTextures, uint32_t numShadowMaps);
	std::function<void(vk::DescriptorPool*)> poolDeleter = [this](vk::DescriptorPool *pool) { if (context->getDevice()) context->getDevice()->destroyDescriptorPool(*pool); };
	std::unique_ptr<vk::DescriptorPool, decltype(poolDeleter)> pool;

	std::function<void(vk::DescriptorSetLayout*)> layoutDeleter = [this](vk::DescriptorSetLayout *layout) { if (context->getDevice()) context->getDevice()->destroyDescriptorSetLayout(*layout); };

	static vk::DescriptorSetLayout *createMaterialLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)> materialLayout;

	static vk::DescriptorSetLayout *createShadowMapLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)> shadowMapLayout;

	static vk::DescriptorSetLayout *createGeometryBufferLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)> geometryBufferLayout;

	static vk::DescriptorSetLayout *createLightingBufferLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)> lightingBufferLayout;

	static vk::DescriptorSetLayout *createFontLayout(const std::shared_ptr<Context> context);
	std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)> fontLayout;

public:
	DescriptorPool(const std::shared_ptr<Context> context, uint32_t numMaterials, uint32_t numShadowMaps);

	vk::DescriptorPool *getPool() const { return pool.get(); }

	vk::DescriptorSetLayout *getMaterialLayout() const { return materialLayout.get(); }
	vk::DescriptorSetLayout *getShadowMapLayout() const { return shadowMapLayout.get(); }
	vk::DescriptorSetLayout *getGeometryBufferLayout() const { return geometryBufferLayout.get(); }
	vk::DescriptorSetLayout *getLightingBufferLayout() const { return lightingBufferLayout.get(); }
	vk::DescriptorSetLayout *getFontLayout() const { return fontLayout.get(); }
};