#include "IndexBuffer.hpp"

void IndexBuffer::finalize(const std::shared_ptr<Context> context)
{
	vk::DeviceSize size = sizeof(indices[0]) * indices.size();

#ifndef MK_OPTIMIZATION_VERTEX_INDEX_BUFFER_STAGING

	buffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eIndexBuffer, size, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	auto memory = context->getDevice()->mapMemory(*buffer->getMemory(), 0, size);
	memcpy(memory, indices.data(), size);
	context->getDevice()->unmapMemory(*buffer->getMemory());

#else

	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	auto commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);

	auto stagingBuffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eTransferSrc, size, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	auto memory = context->getDevice()->mapMemory(*stagingBuffer->getMemory(), 0, size);
	memcpy(memory, indices.data(), size);
	context->getDevice()->unmapMemory(*stagingBuffer->getMemory());

	buffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, size, vk::MemoryPropertyFlagBits::eDeviceLocal);
	commandBuffer.copyBuffer(*stagingBuffer->getBuffer(), *buffer->getBuffer(), vk::BufferCopy(0, 0, size));

	commandBuffer.end();
	auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);
	context->getQueue().submit({ submitInfo }, nullptr);
	context->getQueue().waitIdle();
	context->getDevice()->freeCommandBuffers(*context->getCommandPool(), 1, &commandBuffer);

#endif
}