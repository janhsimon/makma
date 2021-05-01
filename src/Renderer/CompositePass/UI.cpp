#include "UI.hpp"
#include "../Buffers/Buffer.hpp"
#include "../Settings.hpp"
#include "../Shader.hpp"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <imguizmo/imguizmo.h>
#include <ios>
#include <numeric>
#include <sstream>

int UI::windowWidth = Settings::windowWidth;
int UI::windowHeight = Settings::windowHeight;
int UI::windowMode = Settings::windowMode;
int UI::renderMode = Settings::renderMode;
// bool UI::mipMapping = Settings::mipMapping;
float UI::mipLoadBias = Settings::mipLoadBias;
bool UI::reuseCommandBuffers = Settings::reuseCommandBuffers;
bool UI::transientCommandPool = Settings::transientCommandPool;
bool UI::vertexIndexBufferStaging = Settings::vertexIndexBufferStaging;
bool UI::keepUniformBufferMemoryMapped = Settings::keepUniformBufferMemoryMapped;
int UI::dynamicUniformBufferStrategy = Settings::dynamicUniformBufferStrategy;
bool UI::flushDynamicUniformBufferMemoryIndividually = Settings::flushDynamicUniformBufferMemoryIndividually;
int UI::shadowMapResolution = Settings::shadowMapResolution;
int UI::shadowMapCascadeCount = Settings::shadowMapCascadeCount;
float UI::shadowBias = Settings::shadowBias;
int UI::shadowFilterRange = Settings::shadowFilterRange;
float UI::bloomThreshold = Settings::bloomThreshold;
int UI::blurKernelSize = (Settings::blurKernelSize - 1) / 2;
float UI::blurSigma = Settings::blurSigma;
float UI::volumetricIntensity = Settings::volumetricIntensity;
int UI::volumetricSteps = Settings::volumetricSteps;
float UI::volumetricScattering = Settings::volumetricScattering;

ImGuiContext* UI::imGuiContext = nullptr;
std::array<float, 50> UI::totalTime, UI::shadowPassTime, UI::geometryPassTime, UI::lightingPassTime,
  UI::compositePassTime;
std::vector<float> UI::resultTotal, UI::resultShadowPass, UI::resultGeometryPass, UI::resultLightingPass,
  UI::resultCompositePass;

vk::Buffer* UI::createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
  auto bufferCreateInfo = vk::BufferCreateInfo().setSize(size).setUsage(usage);
  auto buffer = context->getDevice()->createBuffer(bufferCreateInfo);
  return new vk::Buffer(buffer);
}

vk::DeviceMemory* UI::createBufferMemory(const std::shared_ptr<Context> context,
                                         const vk::Buffer* buffer,
                                         vk::DeviceSize size,
                                         vk::MemoryPropertyFlags memoryPropertyFlags)
{
  auto memoryRequirements = context->getDevice()->getBufferMemoryRequirements(*buffer);
  auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

  uint32_t memoryTypeIndex = 0;
  bool foundMatch = false;
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
  {
    if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
    {
      memoryTypeIndex = i;
      foundMatch = true;
      break;
    }
  }

  if (!foundMatch)
  {
    throw std::runtime_error("Failed to find suitable memory type for buffer.");
  }

  auto memoryAllocateInfo =
    vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
  auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
  context->getDevice()->bindBufferMemory(*buffer, deviceMemory, 0);
  return new vk::DeviceMemory(deviceMemory);
}

vk::Image* UI::createFontImage(const std::shared_ptr<Context> context, uint32_t width, uint32_t height)
{
  auto imageCreateInfo = vk::ImageCreateInfo()
                           .setImageType(vk::ImageType::e2D)
                           .setExtent(vk::Extent3D(width, height, 1))
                           .setMipLevels(1)
                           .setArrayLayers(1);
  imageCreateInfo.setFormat(vk::Format::eR8G8B8A8Unorm)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
  auto image = context->getDevice()->createImage(imageCreateInfo);
  return new vk::Image(image);
}

vk::DeviceMemory* UI::createFontImageMemory(const std::shared_ptr<Context> context,
                                            const vk::Image* image,
                                            vk::MemoryPropertyFlags memoryPropertyFlags)
{
  auto memoryRequirements = context->getDevice()->getImageMemoryRequirements(*image);
  auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

  uint32_t memoryTypeIndex = 0;
  bool foundMatch = false;
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
  {
    if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
    {
      memoryTypeIndex = i;
      foundMatch = true;
      break;
    }
  }

  if (!foundMatch)
  {
    throw std::runtime_error("Failed to find suitable memory type for font image.");
  }

  auto memoryAllocateInfo =
    vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
  auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
  context->getDevice()->bindImageMemory(*image, deviceMemory, 0);
  return new vk::DeviceMemory(deviceMemory);
}

vk::ImageView* UI::createFontImageView(const std::shared_ptr<Context> context, const vk::Image* image)
{
  auto imageViewCreateInfo = vk::ImageViewCreateInfo()
                               .setImage(*image)
                               .setViewType(vk::ImageViewType::e2D)
                               .setFormat(vk::Format::eR8G8B8A8Unorm);
  imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
  auto fontImageView = context->getDevice()->createImageView(imageViewCreateInfo);
  return new vk::ImageView(fontImageView);
}

vk::Sampler* UI::createSampler(const std::shared_ptr<Context> context)
{
  auto samplerCreateInfo = vk::SamplerCreateInfo()
                             .setMagFilter(vk::Filter::eLinear)
                             .setMinFilter(vk::Filter::eLinear)
                             .setMipmapMode(vk::SamplerMipmapMode::eLinear);
  samplerCreateInfo.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
    .setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
    .setAddressModeW(vk::SamplerAddressMode::eClampToEdge);
  samplerCreateInfo.setMaxAnisotropy(1.0f).setMaxLod(1.0f).setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
  auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
  return new vk::Sampler(sampler);
}

vk::PipelineLayout*
UI::createPipelineLayout(const std::shared_ptr<Context> context, std::vector<vk::DescriptorSetLayout> setLayouts)
{
  auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
                                    .setSetLayoutCount(static_cast<uint32_t>(setLayouts.size()))
                                    .setPSetLayouts(setLayouts.data());
  auto pushConstantRange =
    vk::PushConstantRange().setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(glm::vec2) * 2);
  pipelineLayoutCreateInfo.setPushConstantRangeCount(1);
  pipelineLayoutCreateInfo.setPPushConstantRanges(&pushConstantRange);
  auto pipelineLayout = context->getDevice()->createPipelineLayout(pipelineLayoutCreateInfo);
  return new vk::PipelineLayout(pipelineLayout);
}

vk::Pipeline* UI::createPipeline(const std::shared_ptr<Window> window,
                                 const vk::RenderPass* renderPass,
                                 const vk::PipelineLayout* pipelineLayout,
                                 std::shared_ptr<Context> context)
{
  Shader vertexShader(context, "Shaders/UI.vert.spv", vk::ShaderStageFlagBits::eVertex);
  Shader fragmentShader(context, "Shaders/UI.frag.spv", vk::ShaderStageFlagBits::eFragment);

  std::vector<vk::PipelineShaderStageCreateInfo> pipelineShaderStageCreateInfos = {
    vertexShader.getPipelineShaderStageCreateInfo(), fragmentShader.getPipelineShaderStageCreateInfo()
  };

  auto vertexInputBindingDescription = vk::VertexInputBindingDescription().setStride(sizeof(ImDrawVert));
  auto position = vk::VertexInputAttributeDescription()
                    .setLocation(0)
                    .setFormat(vk::Format::eR32G32Sfloat)
                    .setOffset(offsetof(ImDrawVert, pos));
  auto texCoord = vk::VertexInputAttributeDescription()
                    .setLocation(1)
                    .setFormat(vk::Format::eR32G32Sfloat)
                    .setOffset(offsetof(ImDrawVert, uv));
  auto color = vk::VertexInputAttributeDescription()
                 .setLocation(2)
                 .setFormat(vk::Format::eR8G8B8A8Unorm)
                 .setOffset(offsetof(ImDrawVert, col));
  std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions = { position, texCoord, color };
  auto vertexInputStateCreateInfo =
    vk::PipelineVertexInputStateCreateInfo().setVertexBindingDescriptionCount(1).setPVertexBindingDescriptions(
      &vertexInputBindingDescription);
  vertexInputStateCreateInfo.setVertexAttributeDescriptionCount(
    static_cast<uint32_t>(vertexInputAttributeDescriptions.size()));
  vertexInputStateCreateInfo.setPVertexAttributeDescriptions(vertexInputAttributeDescriptions.data());

  auto inputAssemblyStateCreateInfo =
    vk::PipelineInputAssemblyStateCreateInfo().setTopology(vk::PrimitiveTopology::eTriangleList);

  auto viewport = vk::Viewport().setWidth(1).setHeight(1).setMaxDepth(0.0f);
  auto scissor = vk::Rect2D().setOffset(vk::Offset2D(0, 0)).setExtent(vk::Extent2D(1, 1));
  auto viewportStateCreateInfo =
    vk::PipelineViewportStateCreateInfo().setViewportCount(1).setPViewports(&viewport).setScissorCount(1).setPScissors(
      &scissor);

  auto rasterizationStateCreateInfo =
    vk::PipelineRasterizationStateCreateInfo().setCullMode(vk::CullModeFlagBits::eNone).setLineWidth(1.0f);

  auto multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo().setMinSampleShading(1.0f);

  std::vector<vk::DynamicState> dynamicStateEnables = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
  auto dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
                                  .setDynamicStateCount(static_cast<uint32_t>(dynamicStateEnables.size()))
                                  .setPDynamicStates(dynamicStateEnables.data());

  auto depthStenctilStateCreateInfo =
    vk::PipelineDepthStencilStateCreateInfo().setDepthTestEnable(false).setDepthWriteEnable(false).setDepthCompareOp(
      vk::CompareOp::eLessOrEqual);

  auto colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
                                     .setBlendEnable(true)
                                     .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                                     .setSrcAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
  colorBlendAttachmentState.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
    .setDstAlphaBlendFactor(vk::BlendFactor::eZero);
  colorBlendAttachmentState.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                              vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
  auto colorBlendStateCreateInfo =
    vk::PipelineColorBlendStateCreateInfo().setAttachmentCount(1).setPAttachments(&colorBlendAttachmentState);

  auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
                              .setStageCount(static_cast<uint32_t>(pipelineShaderStageCreateInfos.size()))
                              .setPStages(pipelineShaderStageCreateInfos.data());
  pipelineCreateInfo.setPVertexInputState(&vertexInputStateCreateInfo)
    .setPInputAssemblyState(&inputAssemblyStateCreateInfo)
    .setPViewportState(&viewportStateCreateInfo);
  pipelineCreateInfo.setPRasterizationState(&rasterizationStateCreateInfo)
    .setPMultisampleState(&multisampleStateCreateInfo)
    .setPDepthStencilState(&depthStenctilStateCreateInfo);
  pipelineCreateInfo.setPColorBlendState(&colorBlendStateCreateInfo)
    .setPDynamicState(&dynamicStateCreateInfo)
    .setRenderPass(*renderPass)
    .setLayout(*pipelineLayout);
  auto pipeline = context->getDevice()->createGraphicsPipeline(nullptr, pipelineCreateInfo);
  return new vk::Pipeline(pipeline);
}

vk::DescriptorSet* UI::createDescriptorSet(const std::shared_ptr<Context> context,
                                           const std::shared_ptr<DescriptorPool> descriptorPool,
                                           const vk::ImageView* fontImageView,
                                           const vk::Sampler* sampler)
{
  auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
                                     .setDescriptorPool(*descriptorPool->getPool())
                                     .setDescriptorSetCount(1)
                                     .setPSetLayouts(descriptorPool->getFontLayout());
  auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);

  auto descriptorImageInfo = vk::DescriptorImageInfo()
                               .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                               .setImageView(*fontImageView)
                               .setSampler(*sampler);
  auto samplerWriteDescriptorSet = vk::WriteDescriptorSet()
                                     .setDstBinding(0)
                                     .setDstSet(descriptorSet)
                                     .setDescriptorType(vk::DescriptorType::eCombinedImageSampler);
  samplerWriteDescriptorSet.setDescriptorCount(1).setPImageInfo(&descriptorImageInfo);

  context->getDevice()->updateDescriptorSets(1, &samplerWriteDescriptorSet, 0, nullptr);
  return new vk::DescriptorSet(descriptorSet);
}

void UI::crosshairFrame()
{
  // ImGui::SetNextWindowPosCenter();
  ImGui::SetNextWindowSize(ImVec2(256.0f, 256.0f));
  ImGui::SetNextWindowBgAlpha(0.0f);
  ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
  ImGui::Begin("Crosshair", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  const auto centerX = window->getWidth() / 2.0f;
  const auto centerY = window->getHeight() / 2.0f;
  const auto color = ImColor(ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
  const auto scale = 10.0f;
  const auto gap = 2.0f;

  // horizontal
  drawList->AddLine(ImVec2(centerX - scale + 1.0f, centerY), ImVec2(centerX - gap + 1.0f, centerY), color);
  drawList->AddLine(ImVec2(centerX + scale, centerY), ImVec2(centerX + gap, centerY), color);

  // vertical
  drawList->AddLine(ImVec2(centerX, centerY - scale + 1.0f), ImVec2(centerX, centerY - gap + 1.0f), color);
  drawList->AddLine(ImVec2(centerX, centerY + scale), ImVec2(centerX, centerY + gap), color);

  ImGui::End();

  ImGui::PopStyleColor();
}

void UI::controlsFrame(const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera)
{
  if (input->showControlsWindowKeyPressed)
  {
    ImGui::SetNextWindowPos(ImVec2(300.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    // TODO: remove
    ImGui::Text("Camera Position: %.2f\t%.2f\t%.2f", camera->position.x, camera->position.y, camera->position.z);
    ImGui::Text("Camera Rotation: %.2f\t%.2f\t%.2f", camera->getPitch(), camera->getYaw(), camera->getRoll());

    ImGui::BulletText("TAB to %s mouse cursor.", input->showCursorKeyPressed ? "hide" : "show");
    ImGui::BulletText("MOUSE to look around.");
    ImGui::BulletText("WASD to move.");
    ImGui::BulletText("Hold SHIFT to move slower.");
    ImGui::BulletText("F to %s flying.", input->flyKeyPressed ? "disable" : "enable");

    ImGui::Bullet();
    ImGui::TextColored(input->flyKeyPressed ? ImVec4(0.9f, 0.9f, 0.9f, 1.0f) : ImVec4(0.5f, 0.5f, 0.5f, 1.0f),
                       "SPACE/CTRL while flying to move up/down.");

    ImGui::BulletText("B to %s the benchmark window.", input->showBenchmarkWindowKeyPressed ? "hide" : "show");
    ImGui::BulletText("L to %s the light editor.", input->showLightEditorKeyPressed ? "hide" : "show");
    ImGui::BulletText("G to %s the performance graphs.", input->showGraphsKeyPressed ? "hide" : "show");
    ImGui::BulletText("R to %s the results window.", input->showResultsWindowKeyPressed ? "hide" : "show");
    ImGui::BulletText("C to hide this controls window.");
    ImGui::BulletText("ESC to quit.");

    ImGui::End();
  }
}

void UI::statisticsFrame(const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera, float delta)
{
  const auto distance = 10.0f;
  ImGui::SetNextWindowPos(ImVec2(distance, distance), ImGuiCond_Always, ImVec2(0.0f, 0.0f));
  ImGui::SetNextWindowBgAlpha(0.3f);
  ImGui::Begin("Statistics", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings |
                 ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav);

  // total time
  {
    std::rotate(totalTime.begin(), totalTime.begin() + 1, totalTime.end());
    totalTime.back() = delta;

    const auto average = std::accumulate(totalTime.begin(), totalTime.end(), 0.0f) / 50.0f;
    ImGui::Text("Frames per second: %d", static_cast<int>(1000.0f / std::max(average, 1.0f)));

    if (!input->showGraphsKeyPressed || camera->getState() == CameraState::OnRails)
    {
      ImGui::Text("Frame time: %.1f ms", average);
    }
    else
    {
      ImGui::Separator();
      ImGui::Text("Total frame time: %.1f ms", average);

      const auto max = std::ceil(*std::max_element(totalTime.begin(), totalTime.end()));
      std::stringstream s;
      s << static_cast<int>(max) << " ms";
      ImGui::PlotLines(s.str().c_str(), &totalTime[0], 50, 0, "", 0.0f, max, ImVec2(0, 80));
    }
  }

  if (input->showGraphsKeyPressed && camera->getState() != CameraState::OnRails)
  {
    ImGui::Separator();

    // shadow pass time
    {
      uint32_t begin = 0, end = 0;
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 0, 1, sizeof(uint32_t), &begin, 0,
                                                vk::QueryResultFlagBits());
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 1, 1, sizeof(uint32_t), &end, 0,
                                                vk::QueryResultFlagBits());
      auto shadowPassDelta = end - begin;

      std::rotate(shadowPassTime.begin(), shadowPassTime.begin() + 1, shadowPassTime.end());
      shadowPassTime.back() = static_cast<float>(shadowPassDelta) / 1e6f;

      const auto average = std::accumulate(shadowPassTime.begin(), shadowPassTime.end(), 0.0f) / 50.0f;
      ImGui::Text("Shadow pass time: %.1f ms", average);

      const auto max = std::ceil(*std::max_element(shadowPassTime.begin(), shadowPassTime.end()));
      std::stringstream s;
      s << static_cast<int>(max) << " ms";
      ImGui::PlotLines(s.str().c_str(), &shadowPassTime[0], 50, 0, "", 0.0f, max, ImVec2(0, 80));
    }

    ImGui::Separator();

    // geometry pass time
    {
      uint32_t begin = 0, end = 0;
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 2, 1, sizeof(uint32_t), &begin, 0,
                                                vk::QueryResultFlagBits());
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 3, 1, sizeof(uint32_t), &end, 0,
                                                vk::QueryResultFlagBits());
      auto geometryPassDelta = end - begin;

      std::rotate(geometryPassTime.begin(), geometryPassTime.begin() + 1, geometryPassTime.end());
      geometryPassTime.back() = static_cast<float>(geometryPassDelta) / 1e6f;

      const auto average = std::accumulate(geometryPassTime.begin(), geometryPassTime.end(), 0.0f) / 50.0f;
      ImGui::Text("Geometry pass time: %.1f ms", average);

      const auto max = std::ceil(*std::max_element(geometryPassTime.begin(), geometryPassTime.end()));
      std::stringstream s;
      s << static_cast<int>(max) << " ms";
      ImGui::PlotLines(s.str().c_str(), &geometryPassTime[0], 50, 0, "", 0.0f, max, ImVec2(0, 80));
    }

    ImGui::Separator();

    // lighting pass time
    {
      uint32_t begin = 0, end = 0;
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 4, 1, sizeof(uint32_t), &begin, 0,
                                                vk::QueryResultFlagBits());
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 5, 1, sizeof(uint32_t), &end, 0,
                                                vk::QueryResultFlagBits());
      auto lightingPassDelta = end - begin;

      std::rotate(lightingPassTime.begin(), lightingPassTime.begin() + 1, lightingPassTime.end());
      lightingPassTime.back() = static_cast<float>(lightingPassDelta) / 1e6f;

      const auto average = std::accumulate(lightingPassTime.begin(), lightingPassTime.end(), 0.0f) / 50.0f;
      ImGui::Text("Lighting pass time: %.1f ms", average);

      const auto max = std::ceil(*std::max_element(lightingPassTime.begin(), lightingPassTime.end()));
      std::stringstream s;
      s << static_cast<int>(max) << " ms";
      ImGui::PlotLines(s.str().c_str(), &lightingPassTime[0], 50, 0, "", 0.0f, max, ImVec2(0, 80));
    }

    ImGui::Separator();

    // composite pass time
    {
      uint32_t begin = 0, end = 0;
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 6, 1, sizeof(uint32_t), &begin, 0,
                                                vk::QueryResultFlagBits());
      context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 7, 1, sizeof(uint32_t), &end, 0,
                                                vk::QueryResultFlagBits());
      auto compositePassDelta = end - begin;

      std::rotate(compositePassTime.begin(), compositePassTime.begin() + 1, compositePassTime.end());
      compositePassTime.back() = static_cast<float>(compositePassDelta) / 1e6f;

      const auto average = std::accumulate(compositePassTime.begin(), compositePassTime.end(), 0.0f) / 50.0f;
      ImGui::Text("Composite pass time: %.1f ms", average);

      const auto max = std::ceil(*std::max_element(compositePassTime.begin(), compositePassTime.end()));
      std::stringstream s;
      s << static_cast<int>(max) << " ms";
      ImGui::PlotLines(s.str().c_str(), &compositePassTime[0], 50, 0, "", 0.0f, max, ImVec2(0, 80));
    }
  }

  ImGui::End();
}

bool UI::lightEditorFrame(const std::shared_ptr<Input> input,
                          const std::shared_ptr<Camera> camera,
                          std::vector<std::shared_ptr<Light>>& lightList)
{
  if (input->showLightEditorKeyPressed)
  {
    static int currentLightIndex = 0;
    static auto currentTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
    static auto currentTransformMode = ImGuizmo::MODE::WORLD;
    static bool showGizmo = true;

    const auto frameSize = ImVec2(400.0f, window->getHeight() / 2 - 15.0f);
    ImGui::SetNextWindowPos(ImVec2(window->getWidth() - frameSize.x - 10.0f, window->getHeight() / 2.0f + 5.0f),
                            ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(frameSize, ImGuiCond_FirstUseEver);

    ImGui::Begin("Light Editor", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::Button("Add Directional"))
    {
      auto newLight = std::make_shared<Light>();
      newLight->DirectionalLight(camera->position + camera->getForward() * 25.0f,
                                 glm::vec3(camera->getPitch(), camera->getYaw(), camera->getRoll()), glm::vec3(1.0f),
                                 1.0f, false);
      lightList.push_back(newLight);
      currentLightIndex = static_cast<int>(lightList.size()) - 1;
      ImGui::End();
      return true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Point"))
    {
      auto newLight = std::make_shared<Light>();
      newLight->PointLight(camera->position + camera->getForward() * 25.0f, glm::vec3(1.0f), 500.0f, 1.0f);
      lightList.push_back(newLight);
      currentLightIndex = static_cast<int>(lightList.size()) - 1;
      ImGui::End();
      return true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Add Spot"))
    {
      auto newLight = std::make_shared<Light>();
      newLight->SpotLight(camera->position + camera->getForward() * 25.0f,
                          glm::vec3(camera->getPitch(), camera->getYaw(), camera->getRoll()), glm::vec3(1.0f), 500.0f,
                          1.0f, 45.0f);
      lightList.push_back(newLight);
      currentLightIndex = static_cast<int>(lightList.size()) - 1;
      ImGui::End();
      return true;
    }

    if (lightList.size() <= 0)
    {
      ImGui::End();
      return false;
    }

    const auto light = lightList.at(currentLightIndex);

    ImGui::SameLine();
    if (ImGui::Button("Remove"))
    {
      lightList.erase(lightList.begin() + currentLightIndex);
      currentLightIndex = 0;
      ImGui::End();
      return true;
    }

    ImGui::Separator();

    if (ImGui::Button("<"))
    {
      currentLightIndex--;
      if (currentLightIndex < 0)
      {
        currentLightIndex = static_cast<int>(lightList.size()) - 1;
      }
    }

    ImGui::SameLine();

    ImGui::Text("Light index: %d/%d", currentLightIndex + 1, lightList.size());

    ImGui::SameLine();
    if (ImGui::Button(">"))
    {
      currentLightIndex++;
      if (currentLightIndex >= lightList.size())
      {
        currentLightIndex = 0;
      }
    }

    ImGui::Separator();

    ImGui::Text("Type:");
    ImGui::SameLine();
    if (light->type == LightType::Directional)
    {
      ImGui::Text("Directional");
    }
    else if (light->type == LightType::Point)
    {
      ImGui::Text("Point");
    }
    else
    {
      ImGui::Text("Spot");
    }

    ImGui::Text("Position: %.2f\t%.2f\t%.2f", light->position.x, light->position.y, light->position.z);
    ImGui::Text("Rotation: %.2f\t%.2f\t%.2f", light->getPitch(), light->getYaw(), light->getRoll());

    ImGui::Text("Color:");
    ImGui::SameLine();
    float lightColor[] = { light->color.r, light->color.g, light->color.b };
    if (ImGui::ColorEdit4("Color", (float*)&lightColor,
                          ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha))
    {
      light->color = glm::vec3(lightColor[0], lightColor[1], lightColor[2]);
    }

    if (light->type != LightType::Directional)
    {
      float range = light->getRange();
      if (ImGui::SliderFloat("Range", &range, 0.0f, 2000.0f, "%.1f"))
      {
        light->setRange(range);
      }
    }

    ImGui::SliderFloat("Intensity", &light->intensity, 0.0f, 10.0f, "%.1f");

    if (light->type == LightType::Spot)
    {
      ImGui::SliderFloat("Spot angle", &light->spotAngle, 0.0f, 180.0f, "%.1f");
    }

    ImGui::Separator();

    ImGui::Text("Transform: ");
    ImGui::Checkbox("Show gizmo", &showGizmo);

    if (ImGui::RadioButton("Translate", currentTransformOperation == ImGuizmo::OPERATION::TRANSLATE))
    {
      currentTransformOperation = ImGuizmo::OPERATION::TRANSLATE;
    }

    ImGui::SameLine();

    if (ImGui::RadioButton("Rotate", currentTransformOperation == ImGuizmo::OPERATION::ROTATE))
    {
      currentTransformOperation = ImGuizmo::OPERATION::ROTATE;
    }

    if (ImGui::RadioButton("World", currentTransformMode == ImGuizmo::MODE::WORLD))
    {
      currentTransformMode = ImGuizmo::MODE::WORLD;
    }

    ImGui::SameLine();

    if (ImGui::RadioButton("Local", currentTransformMode == ImGuizmo::MODE::LOCAL))
    {
      currentTransformMode = ImGuizmo::MODE::LOCAL;
    }

    // draw the gizmo

    if (showGizmo)
    {
      glm::mat4 cameraProjectionMatrix = *camera->getProjectionMatrix();
      cameraProjectionMatrix[1][1] *= -1.0f;

      float matrix[4][4];
      float translation[3] = { light->position.x, light->position.y, light->position.z };
      float rotation[3] = { light->getPitch(), light->getYaw(), light->getRoll() };
      float scale[3] = { light->scale.x, light->scale.y, light->scale.z };
      ImGuizmo::RecomposeMatrixFromComponents(translation, rotation, scale, &matrix[0][0]);
      ImGuizmo::Manipulate(glm::value_ptr(*camera->getViewMatrix()), glm::value_ptr(cameraProjectionMatrix),
                           currentTransformOperation, currentTransformMode, &matrix[0][0]);
      ImGuizmo::DecomposeMatrixToComponents(&matrix[0][0], translation, rotation, scale);

      light->position = glm::vec3(translation[0], translation[1], translation[2]);

      light->setPitch(rotation[0]);
      light->setYaw(rotation[1]);
      light->setRoll(rotation[2]);
      light->recalculateAxesFromAngles();

      light->scale = glm::vec3(scale[0], scale[1], scale[2]);
    }

    ImGui::End();
  }

  return false;
}

bool UI::benchmarkFrame(const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera)
{
  auto shouldApplyChanges = false;
  if (input->showBenchmarkWindowKeyPressed)
  {
    const auto frameSize = ImVec2(400.0f, window->getHeight() / 2 - 15.0f);
    ImGui::SetNextWindowPos(ImVec2(window->getWidth() - frameSize.x - 10.0f, 10.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(frameSize, ImGuiCond_FirstUseEver);

    ImGui::Begin("Benchmark", nullptr, ImGuiWindowFlags_NoCollapse);

    if (ImGui::Button("Apply Changes"))
    {
      shouldApplyChanges = true;
    }

    ImGui::SameLine();

    if (ImGui::Button("Start Benchmark"))
    {
      resultTotal.clear();
      resultShadowPass.clear();
      resultGeometryPass.clear();
      resultLightingPass.clear();
      resultCompositePass.clear();

      input->showResultsWindowKeyPressed = true;
      input->showControlsWindowKeyPressed = input->showGraphsKeyPressed = input->showBenchmarkWindowKeyPressed =
        input->showLightEditorKeyPressed = false;

      camera->startRails();
    }

    // TODO: write tool tips for all settings

    if (ImGui::CollapsingHeader("Window"))
    {
      ImGui::SliderInt("Width", &windowWidth, 800, 1920);
      ImGui::SliderInt("Height", &windowHeight, 600, 1080);

      ImGui::Combo("Mode", &windowMode, "Windowed\0Fullscreen\0Borderless\0\0");
    }

    if (ImGui::CollapsingHeader("Input"))
    {
      ImGui::SliderFloat("Mouse sensitivity", &camera->mouseSensitivity, 1.0f, 100.0f, "%.1f");
    }

    if (ImGui::CollapsingHeader("General"))
    {
      ImGui::Combo("Render mode", &renderMode, "Serial\0Parallel\0\0");
      if (ImGui::IsItemHovered())
      {
        std::string tooltip = "The serial render mode renders to the shadow\n";
        tooltip = tooltip.append("maps first, waits for that to finish, and only\n");
        tooltip = tooltip.append("then starts with rendering to the geometry buffer.\n");
        tooltip = tooltip.append("The parallel render mode renders to the shadow\n");
        tooltip = tooltip.append("maps and geometry buffer simultaneously, which\n");
        tooltip = tooltip.append("is possible as they do not depend on each other.");
        ImGui::SetTooltip(tooltip.c_str());
      }

      // ImGui::Checkbox("Mip Mapping", &mipMapping);
      ImGui::Checkbox("Transient command pool", &transientCommandPool);
      ImGui::Checkbox("Reuse command buffers", &reuseCommandBuffers);
    }

    if (ImGui::CollapsingHeader("Memory Management"))
    {
      ImGui::Checkbox("Stage vertex and index buffers", &vertexIndexBufferStaging);
      ImGui::Checkbox("Keep uniform buffer memory mapped", &keepUniformBufferMemoryMapped);

      ImGui::Combo("Dynamic uniform buffer strategy", &dynamicUniformBufferStrategy, "Individual\0Global\0");
      if (ImGui::IsItemHovered())
      {
        std::string tooltip = "The individual dynamic uniform buffer strategy uses\n";
        tooltip = tooltip.append("a unique dynamic uniform buffer for all types of\n");
        tooltip = tooltip.append("data (shadow map split depth, shadow map cascade\n");
        tooltip = tooltip.append("view and projection matrices, geometry world matrices\n");
        tooltip = tooltip.append("and so on). The global strategy uses only one dynamic\n");
        tooltip = tooltip.append("uniform buffer and stores all data there, sequentially.");
        ImGui::SetTooltip(tooltip.c_str());
      }

      if (dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
      {
        ImGui::Checkbox("Flush uniform buffer memory individually", &flushDynamicUniformBufferMemoryIndividually);
      }
    }

    if (ImGui::CollapsingHeader("Shadow Mapping"))
    {
      ImGui::SliderInt("Resolution", &shadowMapResolution, 64, 16384);
      ImGui::SliderInt("Cascade count", &shadowMapCascadeCount, 1, 16);
      ImGui::SliderFloat("Bias", &shadowBias, 0.0f, 0.01f, "%.4f");
      ImGui::SliderInt("Filter Range", &shadowFilterRange, 0, 8);
    }

    if (ImGui::CollapsingHeader("Post FX"))
    {
      ImGui::SliderFloat("Bloom threshold", &bloomThreshold, 0.0f, 1.0f, "%.3f");
      ImGui::SliderInt("Blur kernel size", &blurKernelSize, 0, 24);
      ImGui::SliderFloat("Blur sigma", &blurSigma, 0.1f, 10.0f, "%.1f");
      ImGui::SliderFloat("Volumetric Intensity", &volumetricIntensity, 0.0f, 100.0f, "%.1f");
      ImGui::SliderInt("Volumetric Steps", &volumetricSteps, 0, 100);
      ImGui::SliderFloat("Volumetric Scattering", &volumetricScattering, 0.0f, 1.0f, "%.2f");
    }

    ImGui::End();
  }

  return shouldApplyChanges;
}

void UI::resultsFrame(const std::shared_ptr<Input> input)
{
  if (!input->showResultsWindowKeyPressed)
  {
    return;
  }

  // ImGui::SetNextWindowPosCenter();
  ImGui::Begin("Results", nullptr,
               ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);

  if (resultTotal.size() <= 0 || resultShadowPass.size() <= 0 || resultGeometryPass.size() <= 0 ||
      resultLightingPass.size() <= 0 || resultCompositePass.size() <= 0)
  {
    ImGui::Text("No benchmark data recorded yet.");
    ImGui::Text("Click on \"Start Benchmark\" in the Benchmark window.");
    ImGui::Text("You may have to enable the window by pressing B first.");
    ImGui::Text("");
  }
  else
  {
    float totalMin, totalAvg, totalMax, shadowMin, shadowAvg, shadowMax, geometryMin, geometryAvg, geometryMax,
      lightingMin, lightingAvg, lightingMax, compositeMin, compositeAvg, compositeMax;

    // total time
    {
      ImGui::Text("TOTAL");

      totalMin = *std::min_element(resultTotal.begin(), resultTotal.end());
      totalAvg = std::accumulate(resultTotal.begin(), resultTotal.end(), 0.0f) / resultTotal.size();
      totalMax = *std::max_element(resultTotal.begin(), resultTotal.end());

      ImGui::Text("Frames per second:\tmin: %d\tavg: %d\tmax: %d", static_cast<int>(1000.0f / std::max(totalMax, 1.0f)),
                  static_cast<int>(1000.0f / std::max(totalAvg, 1.0f)),
                  static_cast<int>(1000.0f / std::max(totalMin, 1.0f)));
      ImGui::Text("Frame time:\tmin: %.1f ms\tavg: %.1f ms\tmax: %.1f ms", totalMin, totalAvg, totalMax);

      const auto yAxis = std::ceil(totalMax);
      std::stringstream s;
      s << static_cast<int>(yAxis) << " ms";
      ImGui::PlotLines(s.str().c_str(), resultTotal.data(), static_cast<int>(resultTotal.size()), 0, "", 0.0f, yAxis,
                       ImVec2(400, 80));
    }

    ImGui::Separator();

    // shadow pass
    {
      ImGui::Text("SHADOW PASS");

      shadowMin = *std::min_element(resultShadowPass.begin(), resultShadowPass.end());
      shadowAvg = std::accumulate(resultShadowPass.begin(), resultShadowPass.end(), 0.0f) / resultShadowPass.size();
      shadowMax = *std::max_element(resultShadowPass.begin(), resultShadowPass.end());

      ImGui::Text("Frame time:\tmin: %.1f ms\tavg: %.1f ms\tmax: %.1f ms", shadowMin, shadowAvg, shadowMax);

      const auto yAxis = std::ceil(shadowMax);
      std::stringstream s;
      s << static_cast<int>(yAxis) << " ms";
      ImGui::PlotLines(s.str().c_str(), resultShadowPass.data(), static_cast<int>(resultShadowPass.size()), 0, "", 0.0f,
                       yAxis, ImVec2(400, 80));
    }

    ImGui::Separator();

    // geometry pass
    {
      ImGui::Text("GEOMETRY PASS");

      geometryMin = *std::min_element(resultGeometryPass.begin(), resultGeometryPass.end());
      geometryAvg =
        std::accumulate(resultGeometryPass.begin(), resultGeometryPass.end(), 0.0f) / resultGeometryPass.size();
      geometryMax = *std::max_element(resultGeometryPass.begin(), resultGeometryPass.end());

      ImGui::Text("Frame time:\tmin: %.1f ms\tavg: %.1f ms\tmax: %.1f ms", geometryMin, geometryAvg, geometryMax);

      const auto yAxis = std::ceil(geometryMax);
      std::stringstream s;
      s << static_cast<int>(yAxis) << " ms";
      ImGui::PlotLines(s.str().c_str(), resultGeometryPass.data(), static_cast<int>(resultGeometryPass.size()), 0, "",
                       0.0f, yAxis, ImVec2(400, 80));
    }

    ImGui::Separator();

    // lighting pass
    {
      ImGui::Text("LIGHTING PASS");

      lightingMin = *std::min_element(resultLightingPass.begin(), resultLightingPass.end());
      lightingAvg =
        std::accumulate(resultLightingPass.begin(), resultLightingPass.end(), 0.0f) / resultLightingPass.size();
      lightingMax = *std::max_element(resultLightingPass.begin(), resultLightingPass.end());

      ImGui::Text("Frame time:\tmin: %.1f ms\tavg: %.1f ms\tmax: %.1f ms", lightingMin, lightingAvg, lightingMax);

      const auto yAxis = std::ceil(lightingMax);
      std::stringstream s;
      s << static_cast<int>(yAxis) << " ms";
      ImGui::PlotLines(s.str().c_str(), resultLightingPass.data(), static_cast<int>(resultLightingPass.size()), 0, "",
                       0.0f, yAxis, ImVec2(400, 80));
    }

    ImGui::Separator();

    // composite pass
    {
      ImGui::Text("COMPOSITE PASS");

      compositeMin = *std::min_element(resultCompositePass.begin(), resultCompositePass.end());
      compositeAvg =
        std::accumulate(resultCompositePass.begin(), resultCompositePass.end(), 0.0f) / resultCompositePass.size();
      compositeMax = *std::max_element(resultCompositePass.begin(), resultCompositePass.end());

      ImGui::Text("Frame time:\tmin: %.1f ms\tavg: %.1f ms\tmax: %.1f ms", compositeMin, compositeAvg, compositeMax);

      const auto yAxis = std::ceil(compositeMax);
      std::stringstream s;
      s << static_cast<int>(yAxis) << " ms";
      ImGui::PlotLines(s.str().c_str(), resultCompositePass.data(), static_cast<int>(resultCompositePass.size()), 0, "",
                       0.0f, yAxis, ImVec2(400, 80));
    }

    ImGui::Text("");

    if (ImGui::Button("Export"))
    {
      std::ofstream exportFile;
      exportFile.open("export.csv", std::ios_base::app);
      exportFile << ",Minimum,Average,Maximum" << std::endl;
      exportFile << "Total," << totalMin << "," << totalAvg << "," << totalMax << std::endl;
      exportFile << "Shadow Pass," << shadowMin << "," << shadowAvg << "," << shadowMax << std::endl;
      exportFile << "Geometry Pass," << geometryMin << "," << geometryAvg << "," << geometryMax << std::endl;
      exportFile << "Lighting Pass," << lightingMin << "," << lightingAvg << "," << lightingMax << std::endl;
      exportFile << "Composite Pass," << compositeMin << "," << compositeAvg << "," << compositeMax << std::endl
                 << std::endl;
      exportFile.close();
    }

    ImGui::SameLine();
  }

  if (ImGui::Button("Dismiss"))
  {
    input->showResultsWindowKeyPressed = false;
  }

  ImGui::End();
}

UI::UI(const std::shared_ptr<Window> window,
       const std::shared_ptr<Context> context,
       const std::shared_ptr<DescriptorPool> descriptorPool,
       std::vector<vk::DescriptorSetLayout> setLayouts,
       vk::RenderPass* renderPass)
{
  this->window = window;
  this->context = context;

  vertexCount = indexCount = 0;

  imGuiContext = ImGui::CreateContext();

  // color scheme
  ImGuiStyle& style = ImGui::GetStyle();
  style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
  style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
  style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
  style.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

  // dimensions
  ImGuiIO& io = ImGui::GetIO();

  // create font texture
  unsigned char* fontData;
  int texWidth, texHeight;
  io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
  VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

  std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> stagingBuffer;
  std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> stagingBufferMemory;

  stagingBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(
    createBuffer(context, uploadSize, vk::BufferUsageFlagBits::eTransferSrc), bufferDeleter);
  stagingBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(
    createBufferMemory(context, stagingBuffer.get(), uploadSize,
                       vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent),
    bufferMemoryDeleter);

  auto memory = context->getDevice()->mapMemory(*stagingBufferMemory, 0, uploadSize);
  memcpy(memory, fontData, uploadSize);
  context->getDevice()->unmapMemory(*stagingBufferMemory);

  fontImage = std::unique_ptr<vk::Image, decltype(fontImageDeleter)>(createFontImage(context, texWidth, texHeight),
                                                                     fontImageDeleter);
  fontImageMemory = std::unique_ptr<vk::DeviceMemory, decltype(fontImageMemoryDeleter)>(
    createFontImageMemory(context, fontImage.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), fontImageMemoryDeleter);

  auto commandBufferAllocateInfo =
    vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPoolOnce()).setCommandBufferCount(1);
  auto commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
  auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  commandBuffer.begin(commandBufferBeginInfo);

  auto barrier = vk::ImageMemoryBarrier()
                   .setOldLayout(vk::ImageLayout::eUndefined)
                   .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                   .setImage(*fontImage);
  barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
    .setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
  commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eHost, vk::PipelineStageFlagBits::eTransfer,
                                vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

  auto region = vk::BufferImageCopy()
                  .setImageExtent(vk::Extent3D(texWidth, texHeight, 1))
                  .setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
  commandBuffer.copyBufferToImage(*stagingBuffer, *fontImage, vk::ImageLayout::eTransferDstOptimal, 1, &region);

  barrier = vk::ImageMemoryBarrier()
              .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
              .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
              .setImage(*fontImage);
  barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
  barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eShaderRead);
  commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader,
                                vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

  commandBuffer.end();

  auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);
  context->getQueue().submit({ submitInfo }, nullptr);
  context->getQueue().waitIdle();
  context->getDevice()->freeCommandBuffers(*context->getCommandPoolOnce(), 1, &commandBuffer);

  fontImageView =
    std::unique_ptr<vk::ImageView, decltype(fontImageViewDeleter)>(createFontImageView(context, fontImage.get()),
                                                                   fontImageViewDeleter);
  sampler = std::unique_ptr<vk::Sampler, decltype(samplerDeleter)>(createSampler(context), samplerDeleter);

  descriptorSet = std::unique_ptr<vk::DescriptorSet>(
    createDescriptorSet(context, descriptorPool, fontImageView.get(), sampler.get()));

  pipelineLayout =
    std::unique_ptr<vk::PipelineLayout, decltype(pipelineLayoutDeleter)>(createPipelineLayout(context, setLayouts),
                                                                         pipelineLayoutDeleter);
  pipeline = std::unique_ptr<vk::Pipeline, decltype(pipelineDeleter)>(createPipeline(window, renderPass,
                                                                                     pipelineLayout.get(), context),
                                                                      pipelineDeleter);
}

void UI::makeChangesToSettings()
{
  Settings::windowWidth = windowWidth;
  Settings::windowHeight = windowHeight;
  Settings::windowMode = windowMode;
  Settings::renderMode = renderMode;
  // Settings::mipMapping = mipMapping;
  Settings::reuseCommandBuffers = reuseCommandBuffers;
  Settings::transientCommandPool = transientCommandPool;
  Settings::vertexIndexBufferStaging = vertexIndexBufferStaging;
  Settings::keepUniformBufferMemoryMapped = keepUniformBufferMemoryMapped;
  Settings::dynamicUniformBufferStrategy = dynamicUniformBufferStrategy;
  Settings::flushDynamicUniformBufferMemoryIndividually = flushDynamicUniformBufferMemoryIndividually;
  Settings::shadowMapResolution = shadowMapResolution;
  Settings::shadowMapCascadeCount = shadowMapCascadeCount;
  Settings::shadowBias = shadowBias;
  Settings::shadowFilterRange = shadowFilterRange;
  Settings::bloomThreshold = bloomThreshold;
  Settings::blurKernelSize = blurKernelSize * 2 + 1;
  Settings::blurSigma = blurSigma;
  Settings::volumetricIntensity = volumetricIntensity;
  Settings::volumetricSteps = volumetricSteps;
  Settings::volumetricScattering = volumetricScattering;
}

bool UI::update(const std::shared_ptr<Input> input,
                const std::shared_ptr<Camera> camera,
                std::vector<std::shared_ptr<Light>>& lightList,
                const std::shared_ptr<ShadowPipeline> shadowPipeline,
                const std::shared_ptr<CompositePipeline> compositePipeline,
                const std::shared_ptr<LightingBuffer> lightingBuffer,
                float delta)
{
  if (camera->getState() == CameraState::OnRails)
  {
    resultTotal.push_back(delta);

    uint32_t begin = 0, end = 0;
    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 0, 1, sizeof(uint32_t), &begin, 0,
                                              vk::QueryResultFlagBits());
    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 1, 1, sizeof(uint32_t), &end, 0,
                                              vk::QueryResultFlagBits());
    resultShadowPass.push_back(static_cast<float>(end - begin) / 1e6f);

    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 2, 1, sizeof(uint32_t), &begin, 0,
                                              vk::QueryResultFlagBits());
    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 3, 1, sizeof(uint32_t), &end, 0,
                                              vk::QueryResultFlagBits());
    resultGeometryPass.push_back(static_cast<float>(end - begin) / 1e6f);

    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 4, 1, sizeof(uint32_t), &begin, 0,
                                              vk::QueryResultFlagBits());
    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 5, 1, sizeof(uint32_t), &end, 0,
                                              vk::QueryResultFlagBits());
    resultLightingPass.push_back(static_cast<float>(end - begin) / 1e6f);

    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 6, 1, sizeof(uint32_t), &begin, 0,
                                              vk::QueryResultFlagBits());
    context->getDevice()->getQueryPoolResults(*context->getQueryPool(), 7, 1, sizeof(uint32_t), &end, 0,
                                              vk::QueryResultFlagBits());
    resultCompositePass.push_back(static_cast<float>(end - begin) / 1e6f);
  }

  ImGuiIO& io = ImGui::GetIO();

  io.DisplaySize = ImVec2((float)window->getWidth(), (float)window->getHeight());
  io.DeltaTime = delta;

  if (input->showCursorKeyPressed)
  {
    io.MousePos = ImVec2(input->getMousePosition().x, input->getMousePosition().y);
    io.MouseDown[0] = input->leftMouseButtonPressed;
    io.MouseDown[1] = input->rightMouseButtonPressed;
  }

  ImGui::NewFrame();

  statisticsFrame(input, camera, delta);

  bool benchmarkFrameWantsToApplyChanges = false, lightEditorWantsToApplyChanges = false;
  if (camera->getState() != CameraState::OnRails)
  {
    // crosshairFrame();
    resultsFrame(input);
    controlsFrame(input, camera);
    benchmarkFrameWantsToApplyChanges = benchmarkFrame(input, camera);

    ImGuizmo::BeginFrame();
    ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
    lightEditorWantsToApplyChanges = lightEditorFrame(input, camera, lightList);
  }

  ImGui::Render();

  // update vertex/index buffers

  ImDrawData* imDrawData = ImGui::GetDrawData();

  // alignment is done inside buffer creation
  VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
  VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

  // vertex buffer
  if (!vertexBuffer || vertexCount != imDrawData->TotalVtxCount)
  {
    if (vertexBuffer)
    {
      context->getDevice()->unmapMemory(*vertexBuffer->getMemory());
    }

    vertexBuffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eVertexBuffer, vertexBufferSize,
                                            vk::MemoryPropertyFlagBits::eHostVisible);
    vertexCount = imDrawData->TotalVtxCount;

    vertexBufferMemory = context->getDevice()->mapMemory(*vertexBuffer->getMemory(), 0, vertexBufferSize);
  }

  // index buffer
  if (!indexBuffer || indexCount < imDrawData->TotalIdxCount)
  {
    if (indexBuffer)
    {
      context->getDevice()->unmapMemory(*indexBuffer->getMemory());
    }

    indexBuffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eIndexBuffer, indexBufferSize,
                                           vk::MemoryPropertyFlagBits::eHostVisible);
    indexCount = imDrawData->TotalIdxCount;

    indexBufferMemory = context->getDevice()->mapMemory(*indexBuffer->getMemory(), 0, indexBufferSize);
  }

  // upload data
  ImDrawVert* vtxDst = (ImDrawVert*)vertexBufferMemory;
  ImDrawIdx* idxDst = (ImDrawIdx*)indexBufferMemory;

  for (int n = 0; n < imDrawData->CmdListsCount; n++)
  {
    const ImDrawList* cmd_list = imDrawData->CmdLists[n];
    memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
    memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
    vtxDst += cmd_list->VtxBuffer.Size;
    idxDst += cmd_list->IdxBuffer.Size;
  }

  std::vector<vk::MappedMemoryRange> memoryRanges;
  memoryRanges.push_back(vk::MappedMemoryRange(*vertexBuffer->getMemory(), 0, VK_WHOLE_SIZE));
  memoryRanges.push_back(vk::MappedMemoryRange(*indexBuffer->getMemory(), 0, VK_WHOLE_SIZE));
  context->getDevice()->flushMappedMemoryRanges(static_cast<uint32_t>(memoryRanges.size()), memoryRanges.data());

  return lightEditorWantsToApplyChanges || benchmarkFrameWantsToApplyChanges;
}

void UI::render(const vk::CommandBuffer* commandBuffer)
{
  if (!vertexBuffer || !indexBuffer)
  {
    return;
  }

  ImGuiIO& io = ImGui::GetIO();

  commandBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *pipelineLayout, 0, 1, descriptorSet.get(), 0,
                                    nullptr);

  commandBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, *pipeline);

  VkDeviceSize offsets[] = { 0 };
  commandBuffer->bindVertexBuffers(0, 1, vertexBuffer->getBuffer(), offsets);
  commandBuffer->bindIndexBuffer(*indexBuffer->getBuffer(), 0, vk::IndexType::eUint16);

  auto viewport = vk::Viewport(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, 1.0f);
  commandBuffer->setViewport(0, 1, &viewport);

  struct PushConstants
  {
    glm::vec2 translation;
    glm::vec2 scale;
  } pushConstants;
  pushConstants.translation = glm::vec2(-1.0f);
  pushConstants.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
  commandBuffer->pushConstants(*pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::vec2) * 2,
                               &pushConstants);

  auto imDrawData = ImGui::GetDrawData();
  int32_t vertexOffset = 0;
  int32_t indexOffset = 0;
  for (int32_t i = 0; i < imDrawData->CmdListsCount; ++i)
  {
    const ImDrawList* cmd_list = imDrawData->CmdLists[i];
    for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; ++j)
    {
      const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];

      auto x = std::max((int32_t)(pcmd->ClipRect.x), 0);
      auto y = std::max((int32_t)(pcmd->ClipRect.y), 0);
      auto w = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
      auto h = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);

      auto scissor = vk::Rect2D(vk::Offset2D(x, y), vk::Extent2D(w, h));
      commandBuffer->setScissor(0, 1, &scissor);

      commandBuffer->drawIndexed(pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
      indexOffset += pcmd->ElemCount;
    }

    vertexOffset += cmd_list->VtxBuffer.Size;
  }
}