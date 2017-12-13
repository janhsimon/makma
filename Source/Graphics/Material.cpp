#include "Material.hpp"

std::shared_ptr<Texture> Material::defaultDiffuseTexture, Material::defaultNormalTexture, Material::defaultOpacityTexture, Material::defaultOcclusionTexture;
std::vector<std::shared_ptr<Material>> Material::materials;
uint32_t Material::numMaterials = 0;

vk::DescriptorSet *Material::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const Texture *diffuseTexture, const Texture *normalTexture, const Texture *opacityTexture, const Texture *occlusionTexture)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptor->getDescriptorPool()).setDescriptorSetCount(1).setPSetLayouts(descriptor->getMaterialDescriptorSetLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto diffuseDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*diffuseTexture->getImageView()).setSampler(*diffuseTexture->getSampler());
	auto diffuseSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	diffuseSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&diffuseDescriptorImageInfo);

	auto normalDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*normalTexture->getImageView()).setSampler(*normalTexture->getSampler());
	auto normalSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(1).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&normalDescriptorImageInfo);

	auto opacityDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*opacityTexture->getImageView()).setSampler(*opacityTexture->getSampler());
	auto opacitySamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(2).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	opacitySamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&opacityDescriptorImageInfo);

	auto occlusionDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*occlusionTexture->getImageView()).setSampler(*occlusionTexture->getSampler());
	auto occlusionSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(3).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	occlusionSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&occlusionDescriptorImageInfo);

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { diffuseSamplerWriteDescriptorSet, normalSamplerWriteDescriptorSet, opacitySamplerWriteDescriptorSet, occlusionSamplerWriteDescriptorSet };
	context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

Material::Material(const std::shared_ptr<Context> context, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename, const std::string &opacityTextureFilename, const std::string &occlusionTextureFilename)
{
	this->context = context;
	this->name = name;

	diffuseTexture = diffuseTextureFilename.empty() ? defaultDiffuseTexture : std::make_unique<Texture>(context, "Textures" + diffuseTextureFilename);
	normalTexture = normalTextureFilename.empty() ? defaultNormalTexture : std::make_unique<Texture>(context, "Textures" + normalTextureFilename);
	opacityTexture = opacityTextureFilename.empty() ? defaultOpacityTexture : std::make_unique<Texture>(context, "Textures" + opacityTextureFilename);
	occlusionTexture = occlusionTextureFilename.empty() ? defaultOcclusionTexture : std::make_unique<Texture>(context, "Textures" + occlusionTextureFilename);

	isFinalized = false;
}

void Material::loadDefaultTextures(const std::shared_ptr<Context> context)
{
	defaultDiffuseTexture = std::make_shared<Texture>(context, "Textures\\DefaultDiffuse.tga");
	defaultNormalTexture = std::make_shared<Texture>(context, "Textures\\DefaultNormal.tga");
	defaultOpacityTexture = std::make_shared<Texture>(context, "Textures\\DefaultOpacity.tga");
	defaultOcclusionTexture = std::make_shared<Texture>(context, "Textures\\DefaultOcclusion.tga");
}

std::shared_ptr<Material> Material::cacheMaterial(const std::shared_ptr<Context> context, const std::string &name, std::string &diffuseTextureFilename, const std::string &normalTextureFilename, const std::string &opacityTextureFilename, const std::string &occlusionTextureFilename)
{
	for (auto &material : materials)
	{
		if (material->name.compare(name) == 0)
		{
			return material;
		}
	}

	auto newMaterial = std::make_shared<Material>(context, name, diffuseTextureFilename, normalTextureFilename, opacityTextureFilename, occlusionTextureFilename);
	materials.push_back(newMaterial);
	numMaterials++;
	return newMaterial;
}

void Material::finalize(const std::shared_ptr<Descriptor> descriptor)
{
	if (!isFinalized)
	{
		descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptor, diffuseTexture.get(), normalTexture.get(), opacityTexture.get(), occlusionTexture.get()));
		isFinalized = true;
	}
}