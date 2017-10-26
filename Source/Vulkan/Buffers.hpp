#pragma once

#include "Context.hpp"

#include <glm.hpp>

struct Vertex
{
	glm::vec2 position;
	glm::vec3 color;
};

const std::vector<Vertex> vertices = {
	{ { 0.0f, -0.5f },{ 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, 0.5f },{ 0.0f, 1.0f, 0.0f } },
	{ { -0.5f, 0.5f },{ 0.0f, 0.0f, 1.0f } }
};

/*
const std::vector<Vertex> vertices =
{
	{ { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
	{ { 0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
	{ { 0.5f, 0.5f }, { 0.0f, 0.0f, 1.0f } },
	{ { -0.5f, 0.5f }, { 1.0f, 1.0f, 1.0f } }
};
*/

class Buffers
{
private:
	std::shared_ptr<Context> context;

	static vk::Buffer *createVertexBuffer(const std::shared_ptr<Context> context);
	std::function<void(vk::Buffer*)> vertexBufferDeleter = [this](vk::Buffer *vertexBuffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*vertexBuffer); };
	std::unique_ptr<vk::Buffer, decltype(vertexBufferDeleter)> vertexBuffer;

	static vk::DeviceMemory *createVertexBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer);
	std::function<void(vk::DeviceMemory*)> vertexBufferMemoryDeleter = [this](vk::DeviceMemory *vertexBufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*vertexBufferMemory); };
	std::unique_ptr<vk::DeviceMemory, decltype(vertexBufferMemoryDeleter)> vertexBufferMemory;

public:
	Buffers(std::shared_ptr<Context> context);

	vk::Buffer *getVertexBuffer() const { return vertexBuffer.get(); }
};