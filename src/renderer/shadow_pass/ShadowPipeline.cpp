#include "ShadowPipeline.hpp"
#include "renderer/buffers/VertexBuffer.hpp"
#include "renderer/Settings.hpp"
#include "renderer/Shader.hpp"

vk::RenderPass* ShadowPipeline::createRenderPass(const std::shared_ptr<Context> context)
{
  auto attachmentDescription = vk::AttachmentDescription()
                                 .setLoadOp(vk::AttachmentLoadOp::eClear)
                                 .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
                                 .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare);
  attachmentDescription.setFormat(vk::Format::eD32Sfloat).setFinalLayout(vk::ImageLayout::eDepthStencilReadOnlyOptimal);

  auto depthAttachmentReference =
    vk::AttachmentReference().setAttachment(0).setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
  auto subpassDescription = vk::SubpassDescription()
                              .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
                              .setPDepthStencilAttachment(&depthAttachmentReference);

  std::vector<vk::SubpassDependency> subpassDependencies;
  auto subpassDependency = vk::SubpassDependency()
                             .setSrcSubpass(0)
                             .setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
                             .setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
  subpassDependency.setDstSubpass(0)
    .setDstStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
    .setDstAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
  subpassDependency.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
  subpassDependencies.push_back(subpassDependency);

  auto renderPassCreateInfo = vk::RenderPassCreateInfo()
                                .setAttachmentCount(1)
                                .setPAttachments(&attachmentDescription)
                                .setSubpassCount(1)
                                .setPSubpasses(&subpassDescription);
  renderPassCreateInfo.setDependencyCount(static_cast<uint32_t>(subpassDependencies.size()))
    .setPDependencies(subpassDependencies.data());
  auto renderPass = context->getDevice()->createRenderPass(renderPassCreateInfo);
  return new vk::RenderPass(renderPass);
}

vk::PipelineLayout* ShadowPipeline::createPipelineLayout(const std::shared_ptr<Context> context,
                                                         const std::vector<vk::DescriptorSetLayout> setLayouts)
{
  auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
                                    .setSetLayoutCount(static_cast<uint32_t>(setLayouts.size()))
                                    .setPSetLayouts(setLayouts.data());
  auto pushConstantRange =
    vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(uint32_t));
  pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
  pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
  auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
  return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline* ShadowPipeline::createPipeline(const vk::RenderPass* renderPass,
                                             const vk::PipelineLayout* pipelineLayout,
                                             std::shared_ptr<Context> context)
{
  Shader vertexShader(context, "shaders/ShadowPass.vert.spv", vk::ShaderStageFlagBits::eVertex);
  Shader fragmentShader(context, "shaders/ShadowPass.frag.spv", vk::ShaderStageFlagBits::eFragment);

  auto vertexShaderStageCreateInfo = vertexShader.getPipelineShaderStageCreateInfo();
  auto specializationMapEntry = vk::SpecializationMapEntry().setConstantID(0).setOffset(0).setSize(sizeof(int));
  auto specializationInfo = vk::SpecializationInfo()
                              .setMapEntryCount(1)
                              .setPMapEntries(&specializationMapEntry)
                              .setDataSize(sizeof(int))
                              .setPData(&Settings::shadowMapCascadeCount);
  vertexShaderStageCreateInfo.pSpecializationInfo = &specializationInfo;

  std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = {
    vertexShaderStageCreateInfo, fragmentShader.getPipelineShaderStageCreateInfo()
  };

  auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(Vertex));
  auto position = vk::VertexInputAttributeDescription()
                    .setLocation(0)
                    .setFormat(vk::Format::eR32G32B32Sfloat)
                    .setOffset(offsetof(Vertex, position));
  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position };
  auto vertexInputStateCreateInfo =
    vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(
      &vertexInputBindingDescription);
  vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(
    static_cast<uint32_t>(vertexInputAttributeDescriptions.size()));
  vertexInputStateCreateInfo.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions.data());

  auto inputAssemblyStateCreateInfo =
    vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

  auto viewport = vk::Viewport()
                    .setWidth(static_cast<float>(Settings::shadowMapResolution))
                    .setHeight(static_cast<float>(Settings::shadowMapResolution))
                    .setMaxDepth(1.0f);
  auto scissor = vk::Rect2D().setExtent(vk::Extent2D(Settings::shadowMapResolution, Settings::shadowMapResolution));
  auto viewportStateCreateInfo =
    vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(
      &scissor);

  auto rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
                                        .setCullMode(vk::CullModeFlagBits::eBack)
                                        .setFrontFace(vk::FrontFace::eCounterClockwise)
                                        .setLineWidth(1.0f);

  auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

  auto depthStenctilStateCreateInfo =
    vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(true).setDepthWriteEnable(true).setDepthCompareOp(
      vk::CompareOp::eLessOrEqual);

  auto colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(0);

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

ShadowPipeline::ShadowPipeline(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts)
{
  this->context = context;

  renderPass =
    std::unique_ptr<vk::RenderPass, decltype(renderPassDeleter)>(createRenderPass(context), renderPassDeleter);
  pipelineLayout =
    std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, setLayouts),
                                                                         pipelineLayoutDeleter);
  pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(renderPass.get(),
                                                                                     pipelineLayout.get(), context),
                                                                      pipelineDeleter);
}
