#pragma once

#include "Context.hpp"

#include <glm.hpp>

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

struct UniformBufferData
{
	glm::mat4 cameraViewProjectionMatrix;
	glm::mat4 globals;
};

struct DynamicUniformBufferData
{
	glm::mat4 *shadowMapViewProjectionMatrix;
	glm::mat4 *geometryWorldMatrix;
	glm::mat4 *lightWorldCameraViewProjectionMatrix;
	glm::mat4 *lightData;
};

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

struct ShadowPassDynamicData
{
	glm::mat4 *shadowMapViewProjectionMatrix;
};

struct GeometryPassVertexDynamicData
{
	glm::mat4 *geometryWorldMatrix;
};

struct GeometryPassVertexData
{
	glm::mat4 cameraViewProjectionMatrix;
};

struct LightingPassVertexDynamicData
{
	glm::mat4 *lightWorldCameraViewProjectionMatrix;
};

struct LightingPassVertexData
{
	glm::mat4 globals;
};

struct LightingPassFragmentDynamicData
{
	glm::mat4 *lightData;
};

#endif

class Buffers
{
private:
	std::shared_ptr<Context> context;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	size_t dataAlignment;
	
	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> vertexBuffer, indexBuffer;

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS

std::array<glm::mat4, 3> pushConstants;

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> uniformBuffer;
	UniformBufferData uniformBufferData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> dynamicUniformBuffer;
	DynamicUniformBufferData dynamicUniformBufferData;

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> shadowPassDynamicUniformBuffer;
	ShadowPassDynamicData shadowPassDynamicData;
	
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> geometryPassVertexDynamicUniformBuffer;
	GeometryPassVertexDynamicData geometryPassVertexDynamicData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> geometryPassVertexUniformBuffer;
	GeometryPassVertexData geometryPassVertexData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightingPassVertexDynamicUniformBuffer;
	LightingPassVertexDynamicData lightingPassVertexDynamicData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightingPassVertexUniformBuffer;
	LightingPassVertexData lightingPassVertexData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightingPassFragmentDynamicUniformBuffer;
	LightingPassFragmentDynamicData lightingPassFragmentDynamicData;

#endif
	
	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> vertexBufferMemory, indexBufferMemory;

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> uniformBufferMemory, dynamicUniformBufferMemory;
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> shadowPassDynamicUniformBufferMemory, geometryPassVertexDynamicUniformBufferMemory, geometryPassVertexUniformBufferMemory,
		lightingPassVertexDynamicUniformBufferMemory, lightingPassVertexUniformBufferMemory, lightingPassFragmentDynamicUniformBufferMemory;
#endif

public:
	Buffers(std::shared_ptr<Context> context);

	void finalize(uint32_t numModels, uint32_t numLights, uint32_t numShadowMaps);

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
	vk::Buffer *getIndexBuffer() const { return indexBuffer.get(); }

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS

	std::array<glm::mat4, 3> *getPushConstants() { return &pushConstants; }

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	
	vk::Buffer *getUniformBuffer() const { return uniformBuffer.get(); }
	vk::DeviceMemory *getUniformBufferMemory() const { return uniformBufferMemory.get(); }
	UniformBufferData *getUniformBufferData() { return &uniformBufferData; }

	vk::Buffer *getDynamicUniformBuffer() const { return dynamicUniformBuffer.get(); }
	vk::DeviceMemory *getDynamicUniformBufferMemory() const { return dynamicUniformBufferMemory.get(); }
	DynamicUniformBufferData *getDynamicUniformBufferData() { return &dynamicUniformBufferData; }

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	
	vk::Buffer *getShadowPassVertexDynamicUniformBuffer() const { return shadowPassDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getShadowPassVertexDynamicUniformBufferMemory() const { return shadowPassDynamicUniformBufferMemory.get(); }
	ShadowPassDynamicData *getShadowPassVertexDynamicData() { return &shadowPassDynamicData; }

	vk::Buffer *getGeometryPassVertexDynamicUniformBuffer() const { return geometryPassVertexDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getGeometryPassVertexDynamicUniformBufferMemory() const { return geometryPassVertexDynamicUniformBufferMemory.get(); }
	GeometryPassVertexDynamicData *getGeometryPassVertexDynamicData() { return &geometryPassVertexDynamicData; }

	vk::Buffer *getGeometryPassVertexUniformBuffer() const { return geometryPassVertexUniformBuffer.get(); }
	vk::DeviceMemory *getGeometryPassVertexUniformBufferMemory() const { return geometryPassVertexUniformBufferMemory.get(); }
	GeometryPassVertexData *getGeometryPassVertexData() { return &geometryPassVertexData; }

	vk::Buffer *getLightingPassVertexDynamicUniformBuffer() const { return lightingPassVertexDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getLightingPassVertexDynamicUniformBufferMemory() const { return lightingPassVertexDynamicUniformBufferMemory.get(); }
	LightingPassVertexDynamicData *getLightingPassVertexDynamicData() { return &lightingPassVertexDynamicData; }

	vk::Buffer *getLightingPassVertexUniformBuffer() const { return lightingPassVertexUniformBuffer.get(); }
	vk::DeviceMemory *getLightingPassVertexUniformBufferMemory() const { return lightingPassVertexUniformBufferMemory.get(); }
	LightingPassVertexData *getLightingPassVertexData() { return &lightingPassVertexData; }

	vk::Buffer *getLightingPassFragmentDynamicUniformBuffer() const { return lightingPassFragmentDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getLightingPassFragmentDynamicUniformBufferMemory() const { return lightingPassFragmentDynamicUniformBufferMemory.get(); }
	LightingPassFragmentDynamicData *getLightingPassFragmentDynamicData() { return &lightingPassFragmentDynamicData; }

#endif

	std::vector<Vertex> *getVertices() { return &vertices; }
	std::vector<uint32_t> *getIndices() { return &indices; }

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE != MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS
	size_t getDataAlignment() const { return dataAlignment; }
#endif
};