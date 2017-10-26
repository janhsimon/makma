#include "Buffers.hpp"

vk::Buffer *Buffers::createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	auto bufferCreateInfo = vk::BufferCreateInfo().setSize(size).setUsage(usage);
	auto vertexBuffer = context->getDevice()->createBuffer(bufferCreateInfo);
	return new vk::Buffer(vertexBuffer);
}

vk::DeviceMemory *Buffers::createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, const void *data)
{
	auto memoryPropertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
	auto memoryRequirements = context->getDevice()->getBufferMemoryRequirements(*buffer);
	auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	bool foundMatch = false;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
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

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	
	auto memory = context->getDevice()->mapMemory(deviceMemory, 0, size);
	memcpy(memory, data, size);
	context->getDevice()->unmapMemory(deviceMemory);

	return new vk::DeviceMemory(deviceMemory);
}

Buffers::Buffers(const std::shared_ptr<Context> context)
{
	this->context = context;

	vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
	vertexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, vertexBufferSize, vk::BufferUsageFlagBits::eVertexBuffer), bufferDeleter);
	vertexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, vertexBuffer.get(), vertexBufferSize, vertices.data()), bufferMemoryDeleter);
	context->getDevice()->bindBufferMemory(*vertexBuffer, *vertexBufferMemory, 0);

	vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
	indexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer), bufferDeleter);
	indexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, indexBuffer.get(), indexBufferSize, indices.data()), bufferMemoryDeleter);
	context->getDevice()->bindBufferMemory(*indexBuffer, *indexBufferMemory, 0);
}