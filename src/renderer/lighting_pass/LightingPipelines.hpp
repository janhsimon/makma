#pragma once

#include "renderer/Context.hpp"

class LightingPipelines
{
private:
  std::shared_ptr<Context> context;

  static vk::PipelineLayout*
  createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts);
  std::function<void(vk::PipelineLayout*)> pipelineLayoutDeleter = [this](vk::PipelineLayout* pipelineLayout) {
    if (context->getDevice())
      context->getDevice()->destroyPipelineLayout(*pipelineLayout);
  };
  std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)> pipelineLayoutNoShadowMaps,
    pipelineLayoutWithShadowMaps;

  static vk::Pipeline* createPipelineNoShadowMaps(const std::shared_ptr<Window> window,
                                                  const std::shared_ptr<Context> context,
                                                  const vk::RenderPass* renderPass,
                                                  const vk::PipelineLayout* pipelineLayout);
  static vk::Pipeline* createPipelineWithShadowMaps(const std::shared_ptr<Window> window,
                                                    const std::shared_ptr<Context> context,
                                                    const vk::RenderPass* renderPass,
                                                    const vk::PipelineLayout* pipelineLayout);
  std::function<void(vk::Pipeline*)> pipelineDeleter = [this](vk::Pipeline* pipeline) {
    if (context->getDevice())
      context->getDevice()->destroyPipeline(*pipeline);
  };
  std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)> pipelineNoShadowMaps, pipelineWithShadowMaps;

public:
  LightingPipelines(const std::shared_ptr<Window> window,
                    const std::shared_ptr<Context> context,
                    std::vector<vk::DescriptorSetLayout> setLayoutsNoShadowMaps,
                    std::vector<vk::DescriptorSetLayout> setLayoutsWithShadowMaps,
                    const vk::RenderPass* renderPass);

  vk::PipelineLayout* getPipelineLayoutNoShadowMaps() const
  {
    return pipelineLayoutNoShadowMaps.get();
  }
  vk::Pipeline* getPipelineNoShadowMaps() const
  {
    return pipelineNoShadowMaps.get();
  }

  vk::PipelineLayout* getPipelineLayoutWithShadowMaps() const
  {
    return pipelineLayoutWithShadowMaps.get();
  }
  vk::Pipeline* getPipelineWithShadowMaps() const
  {
    return pipelineWithShadowMaps.get();
  }
};