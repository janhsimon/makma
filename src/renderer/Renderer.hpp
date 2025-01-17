#pragma once

#include "Model.hpp"
#include "Sync.hpp"
#include "core/Camera.hpp"
#include "core/Light.hpp"
#include "renderer/buffers/UniformBuffer.hpp"
#include "renderer/composite_pass/Swapchain.hpp"
#include "renderer/geometry_pass/GeometryBuffer.hpp"
#include "renderer/lighting_pass/LightingBuffer.hpp"
#include "renderer/shadow_pass/ShadowPipeline.hpp"

struct UniformBufferData
{
  glm::mat4 cameraViewProjectionMatrix;
  glm::vec4 cameraPositionNearClip;
  glm::vec4 cameraForwardFarClip;
};

class Renderer
{
private:
  std::shared_ptr<Window> window;
  std::shared_ptr<Input> input;
  std::shared_ptr<Camera> camera;
  std::shared_ptr<Context> context;
  std::shared_ptr<DescriptorPool> descriptorPool;

  std::shared_ptr<VertexBuffer> vertexBuffer;
  std::shared_ptr<IndexBuffer> indexBuffer;

  UniformBufferData uniformBufferData;
  std::shared_ptr<UniformBuffer> uniformBuffer;
  std::shared_ptr<UniformBuffer> dynamicUniformBuffer;
  std::shared_ptr<UniformBuffer> shadowMapSplitDepthsDynamicUniformBuffer,
    shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer, geometryWorldMatrixDynamicUniformBuffer,
    lightWorldMatrixDynamicUniformBuffer, lightDataDynamicUniformBuffer;

  std::vector<std::shared_ptr<Model>> modelList;
  std::vector<std::shared_ptr<Light>> lightList;
  std::shared_ptr<Model> unitQuadModel, unitSphereModel;

  std::shared_ptr<ShadowPipeline> shadowPipeline;

  std::shared_ptr<GeometryBuffer> geometryBuffer;
  std::shared_ptr<GeometryPipeline> geometryPipeline;

  std::shared_ptr<LightingBuffer> lightingBuffer;
  std::shared_ptr<LightingPipelines> lightingPipelines;

  std::shared_ptr<CompositePipeline> compositePipeline;
  std::unique_ptr<Swapchain> swapchain;

  std::shared_ptr<UI> ui;

  std::unique_ptr<Sync> sync;

  uint32_t numShadowMaps;

  void finalizeShadowPass();
  void finalizeGeometryPass();
  void finalizeLightingPass();
  void finalizeCompositePass();

public:
  Renderer(const std::shared_ptr<Window> window,
           const std::shared_ptr<Input> input,
           const std::shared_ptr<Camera> camera);

  std::shared_ptr<Model> loadModel(const std::string& path, const std::string& filename);
  std::shared_ptr<Light> loadDirectionalLight(const glm::vec3& position,
                                              const glm::vec3& eulerAngles,
                                              const glm::vec3& color,
                                              float intensity,
                                              bool castShadows = false);
  std::shared_ptr<Light>
  loadPointLight(const glm::vec3& position, const glm::vec3& color, float range, float intensity);
  std::shared_ptr<Light> loadSpotLight(const glm::vec3& position,
                                       const glm::vec3& eulerAngles,
                                       const glm::vec3& color,
                                       float range,
                                       float intensity,
                                       float cutoffCosine);

  void finalize();
  bool updateUI(float delta);
  void updateBuffers();
  void render();
  // void waitDeviceIdle() const { context->getDevice()->waitIdle(); }
  void waitQueueIdle() const
  {
    context->getQueue().waitIdle();
  }
};