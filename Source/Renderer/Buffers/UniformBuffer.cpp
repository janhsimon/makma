#include "UniformBuffer.hpp"

UniformBuffer::UniformBuffer(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, vk::DeviceSize size, bool dynamic, vk::ShaderStageFlagBits shaderStageFlags, vk::DeviceSize range)
{
	vk::MemoryPropertyFlags memoryPropertyFlagBits = dynamic ? vk::MemoryPropertyFlagBits::eHostVisible : (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
	buffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eUniformBuffer, size, memoryPropertyFlagBits);

	descriptor = std::make_unique<Descriptor>(context, descriptorPool, dynamic ? vk::DescriptorType::eUniformBufferDynamic : vk::DescriptorType::eUniformBuffer, shaderStageFlags, buffer->getBuffer(), range);
}