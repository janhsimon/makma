#pragma once

#include "Context.hpp"

#include <glm.hpp>

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;
};

const std::vector<Vertex> vertices =
{
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};

const std::vector<uint32_t> indices = { 0, 1, 2, 2, 3, 0 };

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

	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> vertexBuffer, indexBuffer, uniformBuffer;

	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> vertexBufferMemory, indexBufferMemory, uniformBufferMemory;

public:
	Buffers(std::shared_ptr<Context> context);

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
	vk::Buffer *getIndexBuffer() const { return indexBuffer.get(); }
	vk::Buffer *getUniformBuffer() const { return uniformBuffer.get(); }
	
	vk::DeviceMemory *getUniformBufferMemory() const { return uniformBufferMemory.get(); }
};