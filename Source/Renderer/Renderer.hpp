#pragma once

#include "Model.hpp"
#include "Semaphores.hpp"
#include "Buffers/UniformBuffer.hpp"
#include "GeometryPass/GeometryBuffer.hpp"
#include "LightingPass/Swapchain.hpp"
#include "ShadowPass/ShadowPipeline.hpp"
#include "../Camera.hpp"
#include "../Light.hpp"

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

struct UniformBufferData
{
	glm::mat4 cameraViewMatrix;
	glm::mat4 cameraProjectionMatrix;
	glm::mat4 globals;
};

/*
struct DynamicUniformBufferData
{
	glm::mat4 *shadowMapSplitDepths;
	//uint32_t *shadowMapCascadeIndex; // index into shadowMapCascadeViewProjectionMatrices during shadow pass
	//glm::mat4 *shadowMapCascadeViewProjectionMatrices;
	glm::mat4 *geometryWorldMatrix;
	glm::mat4 *lightWorldMatrix;
	glm::mat4 *lightData;
};
*/

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

struct ShadowPassDynamicUniformBufferData
{
	glm::mat4 *shadowMapViewProjectionMatrix;
};

struct GeometryPassVertexDynamicUniformBufferData
{
	glm::mat4 *geometryWorldMatrix;
};

struct GeometryPassVertexUniformBufferData
{
	glm::mat4 cameraViewProjectionMatrix;
};

struct LightingPassVertexDynamicUniformBufferData
{
	glm::mat4 *lightWorldCameraViewProjectionMatrix;
};

struct LightingPassVertexUniformBufferData
{
	glm::mat4 globals;
};

struct LightingPassFragmentDynamicUniformBufferData
{
	glm::mat4 *lightData;
};

#endif

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

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	UniformBufferData uniformBufferData;
	//DynamicUniformBufferData dynamicUniformBufferData;
	std::shared_ptr<UniformBuffer> uniformBuffer, dynamicUniformBuffer, shadowMapCascadeDUBO;
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	ShadowPassDynamicUniformBufferData shadowPassDynamicUniformBufferData;
	GeometryPassVertexDynamicUniformBufferData geometryPassVertexDynamicUniformBufferData;
	GeometryPassVertexUniformBufferData geometryPassVertexUniformBufferData;
	LightingPassVertexDynamicUniformBufferData lightingPassVertexDynamicUniformBufferData;
	LightingPassVertexUniformBufferData lightingPassVertexUniformBufferData;
	LightingPassFragmentDynamicUniformBufferData lightingPassFragmentDynamicUniformBufferData;
	std::shared_ptr<UniformBuffer> shadowPassDynamicUniformBuffer, geometryPassVertexDynamicUniformBuffer, geometryPassVertexUniformBuffer, lightingPassVertexDynamicUniformBuffer, lightingPassVertexUniformBuffer, lightingPassFragmentDynamicUniformBuffer;
#endif
	
	std::vector<std::shared_ptr<Model>> modelList;
	std::vector<std::shared_ptr<Light>> lightList;
	std::shared_ptr<Model> unitQuadModel, unitSphereModel;
	std::shared_ptr<ShadowPipeline> shadowPipeline;
	std::shared_ptr<GeometryBuffer> geometryBuffer;
	std::shared_ptr<GeometryPipeline> geometryPipeline;
	std::unique_ptr<Swapchain> swapchain;
	std::shared_ptr<LightingPipeline> lightingPipeline;
	std::unique_ptr<Semaphores> semaphores;
	
public:
	Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera);
	
	std::shared_ptr<Model> loadModel(const std::string &path, const std::string &filename);
	std::shared_ptr<Light> loadLight(LightType type, const glm::vec3 &position, const glm::vec3 &color = glm::vec3(1.0f), float range = 1000.0f, float intensity = 1.0f, float specularPower = 4.0f, bool castShadows = false);

	void finalize();
	void update();
	void render();
	void waitForIdle();
};