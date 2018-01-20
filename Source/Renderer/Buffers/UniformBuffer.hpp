#pragma once

#include "Buffer.hpp"
#include "Descriptor.hpp"

class UniformBuffer
{
private:
	std::unique_ptr<Buffer> buffer;
	std::unique_ptr<Descriptor> descriptor;

public:
	UniformBuffer(const std::shared_ptr<Context> context, const std::shared_ptr<DescriptorPool> descriptorPool, const vk::DeviceSize size, bool dynamic, vk::ShaderStageFlagBits shaderStageFlags, vk::DeviceSize range);

	Buffer *getBuffer() const { return buffer.get(); }
	Descriptor *getDescriptor() const { return descriptor.get(); }
};