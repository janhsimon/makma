#include "Buffers.hpp"

vk::Buffer *Buffers::createVertexBuffer(const std::shared_ptr<Context> context)
{
	auto bufferCreateInfo = vk::BufferCreateInfo().setSize(sizeof(vertices[0]) * vertices.size()).setUsage(vk::BufferUsageFlagBits::eVertexBuffer);
	auto vertexBuffer = context->getDevice()->createBuffer(bufferCreateInfo);
	return new vk::Buffer(vertexBuffer);
}

vk::DeviceMemory *Buffers::createVertexBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer)
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
		throw std::runtime_error("Failed to find suitable memory type for vertex buffer.");
	}

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	
	vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
	auto data = context->getDevice()->mapMemory(deviceMemory, 0, vertexBufferSize);
	memcpy(data, vertices.data(), vertexBufferSize);
	context->getDevice()->unmapMemory(deviceMemory);

	return new vk::DeviceMemory(deviceMemory);
}

Buffers::Buffers(const std::shared_ptr<Context> context)
{
	this->context = context;

	vertexBuffer = std::unique_ptr<vk::Buffer, decltype(vertexBufferDeleter)>(createVertexBuffer(context), vertexBufferDeleter);
	vertexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(vertexBufferMemoryDeleter)>(createVertexBufferMemory(context, vertexBuffer.get()), vertexBufferMemoryDeleter);

	context->getDevice()->bindBufferMemory(*vertexBuffer, *vertexBufferMemory, 0);
}