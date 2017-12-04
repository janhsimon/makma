#pragma once

#include "Descriptor.hpp"
#include "Texture.hpp"

class Material
{
private:
	std::shared_ptr<Context> context;

	static vk::DescriptorSet *createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const Texture *diffuseTexture, const Texture *normalTexture);
	std::unique_ptr<vk::DescriptorSet> descriptorSet;

	std::string name;
	std::unique_ptr<Texture> diffuseTexture, normalTexture;
	bool isFinalized;

	static std::vector<std::shared_ptr<Material>> materials;
	static uint32_t numMaterials;
	
public:
	Material(const std::shared_ptr<Context> context, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename);

	static std::shared_ptr<Material> cacheMaterial(const std::shared_ptr<Context> context, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename);
	static uint32_t getNumMaterials() { return numMaterials; }

	void finalize(const std::shared_ptr<Descriptor> descriptor);

	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); }
};