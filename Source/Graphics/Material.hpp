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

	static std::vector<Material*> materials;
	static uint32_t numMaterials;

	Material(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename);

public:
	static Material *loadMaterial(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename);
	static uint32_t getNumMaterials() { return numMaterials; }

	vk::DescriptorSet *getDescriptorSet() const { return descriptorSet.get(); }
};