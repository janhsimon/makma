#include "CompositePipeline.hpp"
#include "../Buffers/VertexBuffer.hpp"
#include "../Settings.hpp"
#include "../Shader.hpp"

vk::PipelineLayout* CompositePipeline::createPipelineLayout(const std::shared_ptr<Context> context,
                                                            std::vector<vk::DescriptorSetLayout> setLayouts)
{
  auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
                                    .setSetLayoutCount(static_cast<uint32_t>(setLayouts.size()))
                                    .setPSetLayouts(setLayouts.data());
  auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
  return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline* CompositePipeline::createPipeline(const std::shared_ptr<Window> window,
                                                const vk::RenderPass* renderPass,
                                                const vk::PipelineLayout* pipelineLayout,
                                                std::shared_ptr<Context> context)
{
  struct SpecializationData
  {
    int blurKernelSize = Settings::blurKernelSize;
    float blurSigma = Settings::blurSigma;
  } specializationData;

  std::vector<vk::SpecializationMapEntry> specializationConstants;
  specializationConstants.push_back(vk::SpecializationMapEntry()
                                      .setConstantID(0)
                                      .setOffset(offsetof(SpecializationData, blurKernelSize))
                                      .setSize(sizeof(specializationData.blurKernelSize)));
  specializationConstants.push_back(vk::SpecializationMapEntry()
                                      .setConstantID(1)
                                      .setOffset(offsetof(SpecializationData, blurSigma))
                                      .setSize(sizeof(specializationData.blurSigma)));
  auto specializationInfo = vk::SpecializationInfo()
                              .setMapEntryCount(static_cast<uint32_t>(specializationConstants.size()))
                              .setPMapEntries(specializationConstants.data());
  specializationInfo.setDataSize(sizeof(specializationData)).setPData(&specializationData);

  Shader vertexShader(context, "Shaders/CompositePass.vert.spv", vk::ShaderStageFlagBits::eVertex);
  Shader fragmentShader(context, "Shaders/CompositePass.frag.spv", vk::ShaderStageFlagBits::eFragment);

  auto fragmentShaderStageCreateInfo =
    fragmentShader.getPipelineShaderStageCreateInfo().setPSpecializationInfo(&specializationInfo);
  std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = {
    vertexShader.getPipelineShaderStageCreateInfo(), fragmentShaderStageCreateInfo
  };

  auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
  auto position = vk::VertexInputAttributeDescription()
                    .setLocation(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(offsetof(Vertex, position));
  auto vertexInputStateCreateInfo =
    vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(
      &vertexInputBindingDescription);
  vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(1).setPVertexAttributeDescriptions(&position);

  auto inputAssemblyStateCreateInfo =
    vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

  auto viewport = vk::Viewport().setWidth(window->getWidth()).setHeight(window->getHeight()).setMaxDepth(1.0f);
  auto scissor = vk::Rect2D().setExtent(vk::Extent2D(window->getWidth(), window->getHeight()));
  auto viewportStateCreateInfo =
    vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(
      &scissor);

  auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
                                        .setCullMode(vk::CullModeFlagBits::eFront)
                                        .setFrontFace(vk::FrontFace::eCounterClockwise)
                                        .setLineWidth(1.0f);

  auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

  auto depthStenctilStateCreateInfo =
    vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(true).setDepthWriteEnable(true).setDepthCompareOp(
      vk::CompareOp::eLess);

  auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
                                     .setSrcColorBlendFactor(vk::BlendFactor::eOne)
                                     .setSrcAlphaBlendFactor(vk::BlendFactor::eOne);
  colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = { colorBlendAttachmentState };
  auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
                                     .setAttachmentCount(static_cast<uint32_t>(colorBlendAttachmentStates.size()))
                                     .setPAttachments(colorBlendAttachmentStates.data());

  auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
                              .setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size()))
                              .setPStages(pipelineShaderStageCreateInfos.data());
  pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo)
    .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
    .setPViewportState(&viewportStateCreateInfo);
  pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo)
    .setPMultisampleState(&multisampleStateCreateInfo)
    .setPDepthStencilState(&depthStenctilStateCreateInfo);
  pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo);
  pipelineCreateInfo.setRenderPass(*renderPass).setLayout(*pipelineLayout);
  auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
  return new vk::Pipeline(pipeline);
}

CompositePipeline::CompositePipeline(const std::shared_ptr<Window> window,
                                     const std::shared_ptr<Context> context,
                                     std::vector<vk::DescriptorSetLayout> setLayouts,
                                     const vk::RenderPass* renderPass)
{
  this->window = window;
  this->context = context;
  this->renderPass = renderPass;

  pipelineLayout =
    std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, setLayouts),
                                                                         pipelineLayoutDeleter);
  pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass,
                                                                                     pipelineLayout.get(), context),
                                                                      pipelineDeleter);
}
