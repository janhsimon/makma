#pragma once

#include "Buffers/DescriptorPool.hpp"
#include "Texture.hpp"

class Material
{
private:
  std::shared_ptr<Context> context;

  std::string name;

  static vk::DescriptorSet* createDescriptorSet(const std::shared_ptr<Context> context,
                                                const std::shared_ptr<DescriptorPool> descriptorPool,
                                                const Material* material);
  std::unique_ptr<vk::DescriptorSet> descriptorSet;

  std::shared_ptr<Texture> diffuseTexture, normalTexture, metallicTexture, roughnessTexture;

  static std::shared_ptr<Texture> defaultWhiteRGBATexture, defaultBlackRTexture, defaultNormalRGBTexture;
  static std::vector<std::shared_ptr<Material>> materials;
  static uint32_t numMaterials;

public:
  Material(const std::shared_ptr<Context> context, const std::string& name);

  void setDiffuseTexture(const std::string& filename);
  void setNormalTexture(const std::string& filename);
  void setMetallicTexture(const std::string& filename);
  void setRoughnessTexture(const std::string& filename);

  void finalize(const std::shared_ptr<DescriptorPool> descriptorPool);

  vk::DescriptorSet* getDescriptorSet() const
  {
    return descriptorSet.get();
  }

  Texture* getDiffuseTexture() const
  {
    return diffuseTexture.get();
  }
  Texture* getNormalTexture() const
  {
    return normalTexture.get();
  }
  Texture* getMetallicTexture() const
  {
    return metallicTexture.get();
  }
  Texture* getRoughnessTexture() const
  {
    return roughnessTexture.get();
  }

  static void loadDefaultTextures(const std::shared_ptr<Context> context);
  static std::shared_ptr<Material> getMaterialFromCache(const std::string& name);
  static void addMaterialToCache(const std::shared_ptr<Material> material);
  static uint32_t getNumMaterials()
  {
    return numMaterials;
  }
};