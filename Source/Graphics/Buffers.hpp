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

struct WorldData
{
	glm::mat4 *worldMatrix;
};

struct LightData
{
	glm::mat4 *data;
};

struct ViewProjectionData
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

struct EyePositionData
{
	glm::vec3 eyePosition;
};

class Buffers
{
private:
	std::shared_ptr<Context> context;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	size_t worldDataAlignment, lightDataAlignment;
	
	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> vertexBuffer, indexBuffer;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> worldUniformBuffer;
	WorldData worldData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> lightUniformBuffer;
	LightData lightData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> viewProjectionUniformBuffer;
	ViewProjectionData viewProjectionData;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> eyePositionUniformBuffer;
	EyePositionData eyePositionData;
#else
	std::array<glm::mat4, 3> pushConstants;
#endif
	
	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> vertexBufferMemory, indexBufferMemory;

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> worldUniformBufferMemory;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> lightUniformBufferMemory;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> viewProjectionUniformBufferMemory;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> eyePositionUniformBufferMemory;
#endif

public:
	Buffers(std::shared_ptr<Context> context);

	void finalize(uint32_t numModels, uint32_t numLights);

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
	vk::Buffer *getIndexBuffer() const { return indexBuffer.get(); }

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	std::array<glm::mat4, 3> *getPushConstants() { return &pushConstants; }
#else
	vk::Buffer *getWorldUniformBuffer() const { return worldUniformBuffer.get(); }
	vk::DeviceMemory *getWorldUniformBufferMemory() const { return worldUniformBufferMemory.get(); }
	WorldData *getWorldData() { return &worldData; }

	vk::Buffer *getLightUniformBuffer() const { return lightUniformBuffer.get(); }
	vk::DeviceMemory *getLightUniformBufferMemory() const { return lightUniformBufferMemory.get(); }
	LightData *getLightData() { return &lightData; }

	vk::Buffer *getViewProjectionUniformBuffer() const { return viewProjectionUniformBuffer.get(); }
	vk::DeviceMemory *getViewProjectionUniformBufferMemory() const { return viewProjectionUniformBufferMemory.get(); }
	ViewProjectionData *getViewProjectionData() { return &viewProjectionData; }

	vk::Buffer *getEyePositionUniformBuffer() const { return eyePositionUniformBuffer.get(); }
	vk::DeviceMemory *getEyePositionUniformBufferMemory() const { return eyePositionUniformBufferMemory.get(); }
	EyePositionData *getEyePositionData() { return &eyePositionData; }
#endif

	std::vector<Vertex> *getVertices() { return &vertices; }
	std::vector<uint32_t> *getIndices() { return &indices; }

	size_t getWorldDataAlignment() const { return worldDataAlignment; }
	size_t getLightDataAlignment() const { return lightDataAlignment; }
};