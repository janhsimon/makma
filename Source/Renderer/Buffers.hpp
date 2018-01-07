#pragma once

#include "Context.hpp"

#include <glm.hpp>

#define MK_OPTIMIZATION_BUFFER_STAGING

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
	glm::vec3 bitangent;
};

struct ShadowPassVertexDynamicData
{
	glm::mat4 *lightViewProjectionMatrix;
};

struct GeometryPassVertexDynamicData
{
	glm::mat4 *worldMatrix;
};

struct GeometryPassVertexData
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct LightingPassVertexDynamicData
{
	glm::mat4 *worldViewProjectionMatrix;
};

struct LightingPassFragmentDynamicData
{
	glm::mat4 *lightData;
	glm::mat4 *lightViewProjectionMatrix;
};

struct LightingPassFragmentData
{
	glm::mat4 data;
};

class Buffers
{
private:
	std::shared_ptr<Context> context;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	size_t singleMat4DataAlignment, doubleMat4DataAlignment;
	
	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> vertexBuffer, indexBuffer;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> shadowPassVertexDynamicUniformBuffer;
	ShadowPassVertexDynamicData shadowPassVertexDynamicData;
	
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> geometryPassVertexDynamicUniformBuffer;
	GeometryPassVertexDynamicData geometryPassVertexDynamicData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> geometryPassVertexUniformBuffer;
	GeometryPassVertexData geometryPassVertexData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightingPassVertexDynamicUniformBuffer;
	LightingPassVertexDynamicData lightingPassVertexDynamicData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightingPassFragmentDynamicUniformBuffer;
	LightingPassFragmentDynamicData lightingPassFragmentDynamicData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightingPassFragmentUniformBuffer;
	LightingPassFragmentData lightingPassFragmentData;
#else
	std::array<glm::mat4, 3> pushConstants;
#endif
	
	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> vertexBufferMemory, indexBufferMemory;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> shadowPassVertexDynamicUniformBufferMemory, geometryPassVertexDynamicUniformBufferMemory, geometryPassVertexUniformBufferMemory,
		lightingPassVertexDynamicUniformBufferMemory, lightingPassFragmentDynamicUniformBufferMemory, lightingPassFragmentUniformBufferMemory;
#endif

public:
	Buffers(std::shared_ptr<Context> context);

	void finalize(uint32_t numModels, uint32_t numLights, uint32_t numShadowMaps);

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
	vk::Buffer *getIndexBuffer() const { return indexBuffer.get(); }

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::array<glm::mat4, 3> *getPushConstants() { return &pushConstants; }
#else
	vk::Buffer *getShadowPassVertexDynamicUniformBuffer() const { return shadowPassVertexDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getShadowPassVertexDynamicUniformBufferMemory() const { return shadowPassVertexDynamicUniformBufferMemory.get(); }
	ShadowPassVertexDynamicData *getShadowPassVertexDynamicData() { return &shadowPassVertexDynamicData; }

	vk::Buffer *getGeometryPassVertexDynamicUniformBuffer() const { return geometryPassVertexDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getGeometryPassVertexDynamicUniformBufferMemory() const { return geometryPassVertexDynamicUniformBufferMemory.get(); }
	GeometryPassVertexDynamicData *getGeometryPassVertexDynamicData() { return &geometryPassVertexDynamicData; }

	vk::Buffer *getGeometryPassVertexUniformBuffer() const { return geometryPassVertexUniformBuffer.get(); }
	vk::DeviceMemory *getGeometryPassVertexUniformBufferMemory() const { return geometryPassVertexUniformBufferMemory.get(); }
	GeometryPassVertexData *getGeometryPassVertexData() { return &geometryPassVertexData; }

	vk::Buffer *getLightingPassVertexDynamicUniformBuffer() const { return lightingPassVertexDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getLightingPassVertexDynamicUniformBufferMemory() const { return lightingPassVertexDynamicUniformBufferMemory.get(); }
	LightingPassVertexDynamicData *getLightingPassVertexDynamicData() { return &lightingPassVertexDynamicData; }

	vk::Buffer *getLightingPassFragmentDynamicUniformBuffer() const { return lightingPassFragmentDynamicUniformBuffer.get(); }
	vk::DeviceMemory *getLightingPassFragmentDynamicUniformBufferMemory() const { return lightingPassFragmentDynamicUniformBufferMemory.get(); }
	LightingPassFragmentDynamicData *getLightingPassFragmentDynamicData() { return &lightingPassFragmentDynamicData; }

	vk::Buffer *getLightingPassFragmentUniformBuffer() const { return lightingPassFragmentUniformBuffer.get(); }
	vk::DeviceMemory *getLightingPassFragmentUniformBufferMemory() const { return lightingPassFragmentUniformBufferMemory.get(); }
	LightingPassFragmentData *getLightingPassFragmentData() { return &lightingPassFragmentData; }
#endif

	std::vector<Vertex> *getVertices() { return &vertices; }
	std::vector<uint32_t> *getIndices() { return &indices; }

	size_t getSingleMat4DataAlignment() const { return singleMat4DataAlignment; }
	size_t getDoubleMat4DataAlignment() const { return doubleMat4DataAlignment; }
};