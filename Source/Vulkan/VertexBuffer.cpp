#include "Buffer.hpp"
#include "VertexBuffer.hpp"
#include "Vulkan.hpp"
#include "..\Window.hpp"

VertexBuffer::~VertexBuffer()
{
	if (vulkan.device)
	{ 
		if (vulkan.vertexBuffer)
		{
			vulkan.device.destroyBuffer(vulkan.vertexBuffer);
		}
		
		if (vulkan.vertexBufferMemory)
		{
			vulkan.device.freeMemory(vulkan.vertexBufferMemory);
		}
	}
}

bool VertexBuffer::create()
{
	VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingBufferMemory;
	if (!createBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory))
	{
		return false;
	}

	void *data;
	if (vulkan.device.mapMemory(stagingBufferMemory, 0, vertexBufferSize, vk::MemoryMapFlags(), &data) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to map allocated memory for vertex buffer data transfer.");
		return false;
	}

	memcpy(data, vertices.data(), (size_t)vertexBufferSize);
	vulkan.device.unmapMemory(stagingBufferMemory);

	if (!createBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vulkan.vertexBuffer, vulkan.vertexBufferMemory))
	{
		return false;
	}

	if (!copyBuffer(stagingBuffer, vulkan.vertexBuffer, vertexBufferSize))
	{
		return false;
	}

	vulkan.device.destroyBuffer(stagingBuffer);
	vulkan.device.freeMemory(stagingBufferMemory);

	return true;
}