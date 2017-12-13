#pragma once

#include "Descriptor.hpp"
#include "Texture.hpp"

class Material
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const Texture *diffuseTexture, const Texture *normalTexture, const Texture *opacityTexture, const Texture *occlusionTexture);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

	std::string name;
	std::shared_ptr<Texture> diffuseTexture, normalTexture, opacityTexture, occlusionTexture;
	bool isFinalized;

	static std::shared_ptr<Texture> defaultDiffuseTexture, defaultNormalTexture, defaultOpacityTexture, defaultOcclusionTexture;
	static std::vector<std::shared_ptr<Material>> materials;
	static uint32_t numMaterials;
	
public:
	Material(const std::shared_ptr<Context> context, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename, const std::string &opacityTextureFilename, const std::string &occlusionTextureFilename);

	static void loadDefaultTextures(const std::shared_ptr<Context> context);
	static std::shared_ptr<Material> cacheMaterial(const std::shared_ptr<Context> context, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename, const std::string &opacityTextureFilename, const std::string &occlusionTextureFilename);
	static uint32_t getNumMaterials() { return numMaterials; }

	void finalize(const std::shared_ptr<Descriptor> descriptor);

	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); }
};