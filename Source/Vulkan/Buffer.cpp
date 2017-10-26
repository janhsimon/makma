#include "Buffer.hpp"
#include "..\Window.hpp"

bool findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags memoryPropertyFlags, uint32_t &index)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = vulkan.physicalDevice.getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
		{
			index = i;
			return true;
		}
	}

	Window::showMessageBox("Error", "Failed to find suitable memory type for buffer.");
	return false;
}

bool createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer &buffer, vk::DeviceMemory &deviceMemory)
{
	vk::BufferCreateInfo bufferInfo(vk::BufferCreateFlags(), size, usageFlags);
	if (vulkan.device.createBuffer(&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to create buffer.");
		return false;
	}

	vk::MemoryRequirements memoryRequirements = vulkan.device.getBufferMemoryRequirements(buffer);
	
	uint32_t memoryTypeIndex;
	if (!findMemoryType(memoryRequirements.memoryTypeBits, memoryPropertyFlags, memoryTypeIndex))
	{
		return false;
	}

	vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
	if (vulkan.device.allocateMemory(&memoryAllocateInfo, nullptr, &deviceMemory) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to allocate memory for vertex buffer.");
		return false;
	}

	if (vulkan.device.bindBufferMemory(buffer, deviceMemory, 0) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to bind allocated memory to vertex buffer.");
		return false;
	}

	return true;
}

bool copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBuffer commandBuffer;
	vk::CommandBufferAllocateInfo commandBufferAllocateInfo(vulkan.commandPool, vk::CommandBufferLevel::ePrimary, 1);
	if (vulkan.device.allocateCommandBuffers(&commandBufferAllocateInfo, &commandBuffer) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to allocate command buffer for data transfer.");
		return false;
	}

	vk::CommandBufferBeginInfo commandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	if (commandBuffer.begin(commandBufferBeginInfo) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to begin recording command buffer for data transfer.");
		return false;
	}

	commandBuffer.copyBuffer(srcBuffer, dstBuffer, vk::BufferCopy(0, 0, size));
	
	if (commandBuffer.end() != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to end recording command buffer for data transfer.");
		return false;
	}

	vk::SubmitInfo submitInfo(0, nullptr, nullptr, 1, &commandBuffer);
	if (vulkan.queue.submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to submit data transfer command buffer to queue.");
		return false;
	}

	if (vulkan.queue.waitIdle() != vk::Result::eSuccess)
	{
		Window::showMessageBox("Error", "Failed to wait for idle queue.");
		return false;
	}

	vulkan.device.freeCommandBuffers(vulkan.commandPool, 1, &commandBuffer);
	
	return true;
}