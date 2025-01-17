#pragma once

#include "renderer/Texture.hpp"

struct CompositePassSpecializationData
{
  int blurKernelSize;
  float blurSigma;
};

class CompositePipeline
{
private:
  std::shared_ptr<Window> window;
  std::shared_ptr<Context> context;
  const vk::RenderPass* renderPass;

  static vk::PipelineLayout*
  createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts);
  std::function<void(vk::PipelineLayout*)> pipelineLayoutDeleter = [this](vk::PipelineLayout* pipelineLayout) {
    if (context->getDevice())
      context->getDevice()->destroyPipelineLayout(*pipelineLayout);
  };
  std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)> pipelineLayout;

  static vk::Pipeline* createPipeline(const std::shared_ptr<Window> window,
                                      const vk::RenderPass* renderPass,
                                      const vk::PipelineLayout* pipelineLayout,
                                      const std::shared_ptr<Context> context);
  std::function<void(vk::Pipeline*)> pipelineDeleter = [this](vk::Pipeline* pipeline) {
    if (context->getDevice())
      context->getDevice()->destroyPipeline(*pipeline);
  };
  std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)> pipeline;

public:
  CompositePipeline(const std::shared_ptr<Window> window,
                    const std::shared_ptr<Context> context,
                    std::vector<vk::DescriptorSetLayout> setLayouts,
                    const vk::RenderPass* renderPass);

  vk::PipelineLayout* getPipelineLayout() const
  {
    return pipelineLayout.get();
  }
  vk::Pipeline* getPipeline() const
  {
    return pipeline.get();
  }
};