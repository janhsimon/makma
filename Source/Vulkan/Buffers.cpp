#include "Buffers.hpp"

vk::Buffer *Buffers::createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	auto bufferCreateInfo = vk::BufferCreateInfo().setSize(size).setUsage(usage);
	auto buffer = context->getDevice()->createBuffer(bufferCreateInfo);
	return new vk::Buffer(buffer);
}

vk::DeviceMemory *Buffers::createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags, const void *data)
{
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
	context->getDevice()->bindBufferMemory(*buffer, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);
}

#define MK_OPTIMIZATION_BUFFER_STAGING

Buffers::Buffers(const std::shared_ptr<Context> context)
{
	this->context = context;

	vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
	vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

#ifndef MK_OPTIMIZATION_BUFFER_STAGING
	vertexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, vertexBufferSize, vk::BufferUsageFlagBits::eVertexBuffer), bufferDeleter);
	vertexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, vertexBuffer.get(), vertexBufferSize, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertices.data()), bufferMemoryDeleter);
		
	auto memory = context->getDevice()->mapMemory(*vertexBufferMemory, 0, vertexBufferSize);
	memcpy(memory, vertices.data(), vertexBufferSize);
	context->getDevice()->unmapMemory(*vertexBufferMemory);

	indexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, indexBufferSize, vk::BufferUsageFlagBits::eIndexBuffer), bufferDeleter);
	indexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, indexBuffer.get(), indexBufferSize, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indices.data()), bufferMemoryDeleter);

	memory = context->getDevice()->mapMemory(*indexBufferMemory, 0, indexBufferSize);
	memcpy(memory, indices.data(), indexBufferSize);
	context->getDevice()->unmapMemory(*indexBufferMemory);
#else
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> stagingVertexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc), bufferDeleter);
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> stagingVertexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, stagingVertexBuffer.get(), vertexBufferSize, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, vertices.data()), bufferMemoryDeleter);

	auto memory = context->getDevice()->mapMemory(*stagingVertexBufferMemory, 0, vertexBufferSize);
	memcpy(memory, vertices.data(), vertexBufferSize);
	context->getDevice()->unmapMemory(*stagingVertexBufferMemory);

	vertexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer), bufferDeleter);
	vertexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, vertexBuffer.get(), vertexBufferSize, vk::MemoryPropertyFlagBits::eDeviceLocal, vertices.data()), bufferMemoryDeleter);

	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	auto commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);
	commandBuffer.copyBuffer(*stagingVertexBuffer, *vertexBuffer, vk::BufferCopy(0, 0, vertexBufferSize));
	commandBuffer.end();
	auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);

	if (context->getQueue().submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit queue.");
	}

	context->getQueue().waitIdle();
	context->getDevice()->freeCommandBuffers(*context->getCommandPool(), 1, &commandBuffer);
	
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> stagingIndexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc), bufferDeleter);
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> stagingIndexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, stagingIndexBuffer.get(), indexBufferSize, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, indices.data()), bufferMemoryDeleter);

	memory = context->getDevice()->mapMemory(*stagingIndexBufferMemory, 0, indexBufferSize);
	memcpy(memory, indices.data(), indexBufferSize);
	context->getDevice()->unmapMemory(*stagingIndexBufferMemory);

	indexBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, indexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer), bufferDeleter);
	indexBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, indexBuffer.get(), indexBufferSize, vk::MemoryPropertyFlagBits::eDeviceLocal, indices.data()), bufferMemoryDeleter);

	commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
	commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);
	commandBuffer.copyBuffer(*stagingIndexBuffer, *indexBuffer, vk::BufferCopy(0, 0, indexBufferSize));
	commandBuffer.end();
	submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);

	if (context->getQueue().submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit queue.");
	}

	context->getQueue().waitIdle();
	context->getDevice()->freeCommandBuffers(*context->getCommandPool(), 1, &commandBuffer);

#endif
}