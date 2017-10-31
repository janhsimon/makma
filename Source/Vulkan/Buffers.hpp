#pragma once

#include "Context.hpp"

#include <glm.hpp>

#define MK_OPTIMIZATION_BUFFER_STAGING

struct Vertex
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec2 texCoord;
};

struct UniformBufferObject
{
	glm::mat4 worldMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

class Buffers
{
private:
	std::shared_ptr<Context> context;

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	
	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> vertexBuffer, indexBuffer, uniformBuffer;
	
	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> vertexBufferMemory, indexBufferMemory, uniformBufferMemory;

public:
	Buffers(std::shared_ptr<Context> context);

	void finalize();

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
	vk::Buffer *getIndexBuffer() const { return indexBuffer.get(); }
	vk::Buffer *getUniformBuffer() const { return uniformBuffer.get(); }
	
	vk::DeviceMemory *getUniformBufferMemory() const { return uniformBufferMemory.get(); }

	std::vector<Vertex> *getVertices() { return &vertices; }
	std::vector<uint32_t> *getIndices() { return &indices; }
};