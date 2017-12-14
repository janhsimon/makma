#include "Material.hpp"

std::shared_ptr<Texture> Material::defaultWhiteTexture, Material::defaultNormalTexture;
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

	std::vector<vk::WriteDescriptorSet> writeDescriptorSets = { diffuseSamplerWriteDescriptorSet, normalSamplerWriteDescriptorSet, occlusionSamplerWriteDescriptorSet };
	context->getDevice()->updateDescriptorSets(static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	return new vk::DescriptorSet(descriptorSet);
}

Material::Material(const std::shared_ptr<Context> context, const std::string &name)
{
	this->context = context;
	this->name = name;

	diffuseTexture = defaultWhiteTexture;
	normalTexture = defaultNormalTexture;
	occlusionTexture = defaultWhiteTexture;

	isFinalized = false;
}

void Material::setDiffuseTexture(const std::string &filename)
{
	diffuseTexture = Texture::cacheTexture(context, filename);
}

void Material::setNormalTexture(const std::string &filename)
{
	normalTexture = Texture::cacheTexture(context, filename);
}

void Material::setOcclusionTexture(const std::string &filename)
{
	occlusionTexture = Texture::cacheTexture(context, filename);
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
	defaultWhiteTexture = std::make_shared<Texture>(context, "Textures\\DefaultWhite.tga");
	defaultNormalTexture = std::make_shared<Texture>(context, "Textures\\DefaultNormal.tga");
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

void Material::addMaterialToCache(const std::shared_ptr<Context> context, const std::shared_ptr<Material> material)
{
	materials.push_back(material);
	numMaterials++;
}