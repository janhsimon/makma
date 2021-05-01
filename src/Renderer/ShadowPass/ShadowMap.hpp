#pragma once

#include "../../Camera.hpp"
#include "../Buffers/UniformBuffer.hpp"
#include "../Model.hpp"
#include "ShadowPipeline.hpp"

class ShadowMap
{
private:
  std::shared_ptr<Context> context;
  std::shared_ptr<DescriptorPool> descriptorPool;

  static vk::Image* createDepthImage(const std::shared_ptr<Context> context);
  std::function<void(vk::Image*)> depthImageDeleter = [this](vk::Image* depthImage) {
    if (context->getDevice())
      context->getDevice()->destroyImage(*depthImage);
  };
  std::unique_ptr<vk::Image, decltype(depthImageDeleter)> depthImage;

  static vk::DeviceMemory* createDepthImageMemory(const std::shared_ptr<Context> context,
                                                  const vk::Image* image,
                                                  vk::MemoryPropertyFlags memoryPropertyFlags);
  std::function<void(vk::DeviceMemory*)> depthImageMemoryDeleter = [this](vk::DeviceMemory* depthImageMemory) {
    if (context->getDevice())
      context->getDevice()->freeMemory(*depthImageMemory);
  };
  std::unique_ptr<vk::DeviceMemory, decltype(depthImageMemoryDeleter)> depthImageMemory;

  static vk::ImageView* createSharedDepthImageView(const std::shared_ptr<Context> context, const vk::Image* image);
  std::function<void(vk::ImageView*)> sharedDepthImageViewDeleter = [this](vk::ImageView* depthImageView) {
    if (context->getDevice())
      context->getDevice()->destroyImageView(*depthImageView);
  };
  std::unique_ptr<vk::ImageView, decltype(sharedDepthImageViewDeleter)> sharedDepthImageView;

  static std::vector<vk::ImageView>*
  createDepthImageViews(const std::shared_ptr<Context> context, const vk::Image* image);
  std::function<void(std::vector<vk::ImageView>*)> depthImageViewsDeleter =
    [this](std::vector<vk::ImageView>* depthImageViews) {
      if (context->getDevice())
      {
        for (auto& depthImageView : *depthImageViews)
          context->getDevice()->destroyImageView(depthImageView);
      }
    };
  std::unique_ptr<std::vector<vk::ImageView>, decltype(depthImageViewsDeleter)> depthImageViews;

  static std::vector<vk::Framebuffer>* createFramebuffers(const std::shared_ptr<Context> context,
                                                          const std::vector<vk::ImageView>* depthImageViews,
                                                          const vk::RenderPass* renderPass);
  std::function<void(std::vector<vk::Framebuffer>*)> framebuffersDeleter =
    [this](std::vector<vk::Framebuffer>* framebuffers) {
      if (context->getDevice())
      {
        for (auto& framebuffer : *framebuffers)
          context->getDevice()->destroyFramebuffer(framebuffer);
      }
    };
  std::unique_ptr<std::vector<vk::Framebuffer>, decltype(framebuffersDeleter)> framebuffers;

  static vk::Sampler* createSampler(const std::shared_ptr<Context> context);
  std::function<void(vk::Sampler*)> samplerDeleter = [this](vk::Sampler* sampler) {
    if (context->getDevice())
      context->getDevice()->destroySampler(*sampler);
  };
  std::unique_ptr<vk::Sampler, decltype(samplerDeleter)> sampler;

  static vk::CommandBuffer* createCommandBuffer(const std::shared_ptr<Context> context);
  std::unique_ptr<vk::CommandBuffer> commandBuffer;

  static vk::DescriptorSet* createSharedDescriptorSet(const std::shared_ptr<Context> context,
                                                      const std::shared_ptr<DescriptorPool> descriptorPool,
                                                      const ShadowMap* shadowMap);
  std::unique_ptr<vk::DescriptorSet> sharedDescriptorSet;

  static std::vector<vk::DescriptorSet>* createDescriptorSets(const std::shared_ptr<Context> context,
                                                              const std::shared_ptr<DescriptorPool> descriptorPool,
                                                              const ShadowMap* shadowMap);
  std::unique_ptr<std::vector<vk::DescriptorSet>> descriptorSets;

  std::vector<float> splitDepths;
  std::vector<glm::mat4> cascadeViewProjectionMatrices;

public:
  ShadowMap(const std::shared_ptr<Context> context,
            const std::shared_ptr<DescriptorPool> descriptorPool,
            const std::shared_ptr<ShadowPipeline> shadowPipeline);
  ~ShadowMap();

  void recordCommandBuffer(const std::shared_ptr<VertexBuffer> vertexBuffer,
                           const std::shared_ptr<IndexBuffer> indexBuffer,
                           const vk::DescriptorSet* shadowMapCascadeViewProjectionMatricesDescriptorSet,
                           const vk::DescriptorSet* geometryWorldMatrixDescriptorSet,
                           const std::shared_ptr<ShadowPipeline> shadowPipeline,
                           const std::vector<std::shared_ptr<Model>>* models,
                           uint32_t shadowMapIndex,
                           uint32_t numShadowMaps);

  void update(const std::shared_ptr<Camera> camera, const glm::vec3 lightDirection);

  vk::CommandBuffer* getCommandBuffer() const
  {
    return commandBuffer.get();
  }
  vk::DescriptorSet* getSharedDescriptorSet() const
  {
    return sharedDescriptorSet.get();
  }
  vk::DescriptorSet* getDescriptorSet(const uint32_t index) const
  {
    return &descriptorSets->at(index);
  }
  float* getSplitDepths()
  {
    return splitDepths.data();
  }
  glm::mat4* getCascadeViewProjectionMatrices()
  {
    return cascadeViewProjectionMatrices.data();
  }
};
