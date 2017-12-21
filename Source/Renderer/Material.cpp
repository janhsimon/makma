#include "Material.hpp"

std::shared_ptr<Texture> Material::defaultWhiteRGBATexture, Material::defaultWhiteRTexture, Material::defaultBlackRTexture, Material::defaultNormalRGBTexture;
std::vector<std::shared_ptr<Material>> Material::materials;
uint32_t Material::numMaterials = 0;

vk::DescriptorSet *Material::createDescriptorSet(const std::shared_ptr<Context> context, const std::shared_ptr<Descriptor> descriptor, const Material *material)
{
	auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo().setDescriptorPool(*descriptor->getDescriptorPool()).setDescriptorSetCount(1).setPSetLayouts(descriptor->getMaterialDescriptorSetLayout());
	auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

	auto diffuseDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*material->getDiffuseTexture()->getImageView()).setSampler(*material->getDiffuseTexture()->getSampler());
	auto diffuseSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(0).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	diffuseSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&diffuseDescriptorImageInfo);

	auto normalDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*material->getNormalTexture()->getImageView()).setSampler(*material->getNormalTexture()->getSampler());
	auto normalSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(1).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	normalSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&normalDescriptorImageInfo);

	auto occlusionDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*material->getOcclusionTexture()->getImageView()).setSampler(*material->getOcclusionTexture()->getSampler());
	auto occlusionSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(2).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	occlusionSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&occlusionDescriptorImageInfo);

	auto metallicDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*material->getMetallicTexture()->getImageView()).setSampler(*material->getMetallicTexture()->getSampler());
	auto metallicSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(3).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	metallicSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&metallicDescriptorImageInfo);

	auto roughnessDescriptorImageInfo = vk::DescriptorImageInfo().setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImageView(*material->getRoughnessTexture()->getImageView()).setSampler(*material->getRoughnessTexture()->getSampler());
	auto roughnessSamplerWriteDescriptorSet = vk::WriteDescriptorSet().setDstBinding(4).setDstSet(descriptorSet).setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
	roughnessSamplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&roughnessDescriptorImageInfo);

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { diffuseSamplerWriteDescriptorSet, normalSamplerWriteDescriptorSet, occlusionSamplerWriteDescriptorSet, metallicSamplerWriteDescriptorSet, roughnessSamplerWriteDescriptorSet };
	context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

Material::Material(const std::shared_ptr<Context> context, const std::string &name)
{
	this->context = context;
	this->name = name;

	diffuseTexture = defaultWhiteRGBATexture;
	normalTexture = defaultNormalRGBTexture;
	occlusionTexture = defaultWhiteRTexture;
	metallicTexture = defaultBlackRTexture;
	roughnessTexture = defaultBlackRTexture;

	isFinalized = false;
}

void Material::setDiffuseTexture(const std::string &filename)
{
	diffuseTexture = Texture::cacheTexture(context, filename, vk::Format::eR8G8B8A8Unorm);
}

void Material::setNormalTexture(const std::string &filename)
{
	normalTexture = Texture::cacheTexture(context, filename, vk::Format::eR8G8B8A8Unorm);
}

void Material::setOcclusionTexture(const std::string &filename)
{
	occlusionTexture = Texture::cacheTexture(context, filename, vk::Format::eR8Unorm);
}

void Material::setMetallicTexture(const std::string &filename)
{
	metallicTexture = Texture::cacheTexture(context, filename, vk::Format::eR8Unorm);
}

void Material::setRoughnessTexture(const std::string &filename)
{
	roughnessTexture = Texture::cacheTexture(context, filename, vk::Format::eR8Unorm);
}

void Material::finalize(const std::shared_ptr<Descriptor> descriptor)
{
	if (!isFinalized)
	{
		descriptorSet = std::unique_ptr<vk::DescriptorSet>(createDescriptorSet(context, descriptor, this));
		isFinalized = true;
	}
}

void Material::loadDefaultTextures(const std::shared_ptr<Context> context)
{
	defaultWhiteRGBATexture = std::make_shared<Texture>(context, "Textures\\DefaultWhite.tga", vk::Format::eR8G8B8A8Unorm);
	defaultWhiteRTexture = std::make_shared<Texture>(context, "Textures\\DefaultWhite.tga", vk::Format::eR8Unorm);
	defaultBlackRTexture = std::make_shared<Texture>(context, "Textures\\DefaultBlack.tga", vk::Format::eR8Unorm);
	defaultNormalRGBTexture = std::make_shared<Texture>(context, "Textures\\DefaultNormal.tga", vk::Format::eR8G8B8A8Unorm);
}

std::shared_ptr<Material> Material::getMaterialFromCache(const std::string &name)
{
	for (auto &material : materials)
	{
		if (material->name.compare(name) == 0)
		{
			return material;
		}
	}

	return nullptr;
}

void Material::addMaterialToCache(const std::shared_ptr<Material> material)
{
	materials.push_back(material);
	numMaterials++;
}