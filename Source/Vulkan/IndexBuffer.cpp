#include "Buffer.hpp"
#include "IndexBuffer.hpp"
#include "Vulkan.hpp"
#include "..\Window.hpp"

IndexBuffer::~IndexBuffer()
{
	if (vulkan.device)
	{
		if (vulkan.indexBuffer)
		{
			vulkan.device.destroyBuffer(vulkan.indexBuffer);
		}

		if (vulkan.indexBufferMemory)
		{
			vulkan.device.freeMemory(vulkan.indexBufferMemory);
		}
	}
}

bool IndexBuffer::create()
{
	VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	if (!createBuffer(indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory))
	{
		return false;
	}

	void *data;
	if (vulkan.device.mapMemory(stagingBufferMemory, 0, indexBufferSize, vk::MemoryMapFlags(), &data) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to map allocated memory for index buffer data transfer.");
		return false;
	}

	memcpy(data, indices.data(), (size_t)indexBufferSize);
	vulkan.device.unmapMemory(stagingBufferMemory);

	if (!createBuffer(indexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkan.indexBuffer, vulkan.indexBufferMemory))
	{
		return false;
	}

	if (!copyBuffer(stagingBuffer, vulkan.indexBuffer, indexBufferSize))
	{
		return false;
	}

	vulkan.device.destroyBuffer(stagingBuffer);
	vulkan.device.freeMemory(stagingBufferMemory);

	return true;
}