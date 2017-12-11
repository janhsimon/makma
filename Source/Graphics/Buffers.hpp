#pragma once

#include "Context.hpp"
//#include "..\Logic\DirectionalLight.hpp"

#include <glm.hpp>

#define MK_OPTIMIZATION_BUFFER_STAGING

struct Vertex
{
	glm::vec3 position;
	glm::vec2 texCoord;
	glm::vec3 normal;
	glm::vec3 tangent;
};

struct DynamicUniformBufferData
{
	glm::mat4 *worldMatrix;
};

struct ViewProjectionData
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct LightData
{
	//DirectionalLight *directionalLights;
	glm::vec4 directionalLightsDirection[4];
	glm::vec4 directionalLightsColor[4];
};

class Buffers
{
private:
	std::shared_ptr<Context> context;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	size_t dynamicAlignment;
	
	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> vertexBuffer, indexBuffer;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> dynamicUniformBuffer;
	DynamicUniformBufferData dynamicUniformBufferData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> viewProjectionUniformBuffer;
	ViewProjectionData viewProjectionData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightUniformBuffer;
	LightData lightData;
#else
	std::array<glm::mat4, 3> pushConstants;
#endif
	
	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> vertexBufferMemory, indexBufferMemory;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> dynamicUniformBufferMemory;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> viewProjectionUniformBufferMemory;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> lightUniformBufferMemory;
#endif

public:
	Buffers(std::shared_ptr<Context> context);

	void finalize(uint32_t numModels);

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
	vk::Buffer *getIndexBuffer() const { return indexBuffer.get(); }

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::array<glm::mat4, 3> *getPushConstants() { return &pushConstants; }
#else
	vk::Buffer *getDynamicUniformBuffer() const { return dynamicUniformBuffer.get(); }
	vk::DeviceMemory *getDynamicUniformBufferMemory() const { return dynamicUniformBufferMemory.get(); }
	DynamicUniformBufferData *getDynamicUniformBufferData() { return &dynamicUniformBufferData; }

	vk::Buffer *getViewProjectionUniformBuffer() const { return viewProjectionUniformBuffer.get(); }
	vk::DeviceMemory *getViewProjectionUniformBufferMemory() const { return viewProjectionUniformBufferMemory.get(); }
	ViewProjectionData *getViewProjectionData() { return &viewProjectionData; }

	vk::Buffer *getLightUniformBuffer() const { return lightUniformBuffer.get(); }
	vk::DeviceMemory *getLightUniformBufferMemory() const { return lightUniformBufferMemory.get(); }
	LightData *getLightData() { return &lightData; }
#endif

	std::vector<Vertex> *getVertices() { return &vertices; }
	std::vector<uint32_t> *getIndices() { return &indices; }

	size_t getDynamicAlignment() const { return dynamicAlignment; }
};