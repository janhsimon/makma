#pragma once

#include "../Context.hpp"

class Buffer
{
private:
	std::shared_ptr<Context> context;

	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };
	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> buffer;

	static vk::DeviceMemory *createMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> memoryDeleter = [this](vk::DeviceMemory *memory) { if (context->getDevice()) context->getDevice()->freeMemory(*memory); };
	std::unique_ptr<vk::DeviceMemory, decltype(memoryDeleter)> memory;

	bool isMemoryMapped;
	void *mappedMemoryLocation;

public:
	Buffer(const std::shared_ptr<Context> context, vk::BufferUsageFlags usage, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	~Buffer() { unmapMemory(); }

	void mapMemory();
	void unmapMemory();

	vk::Buffer *getBuffer() const { return buffer.get(); }
	vk::DeviceMemory *getMemory() const { return memory.get(); }
	void *getMemoryMappedLocation() const { return mappedMemoryLocation; }
};