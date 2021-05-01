#include "UniformBuffer.hpp"

UniformBuffer::UniformBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, bool dynamic)
{
  this->context = context;
  this->dynamic = dynamic;

  vk::MemoryPropertyFlags memoryPropertyFlagBits =
    dynamic ? vk::MemoryPropertyFlagBits::eHostVisible :
              (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
  buffer = std::make_unique<Buffer>(context, vk::BufferUsageFlagBits::eUniformBuffer, size, memoryPropertyFlagBits);

  descriptors = std::make_unique<std::vector<std::unique_ptr<Descriptor>>>();
}

void UniformBuffer::addDescriptor(const std::shared_ptr<DescriptorPool> descriptorPool,
                                  vk::ShaderStageFlagBits shaderStageFlags,
                                  vk::DeviceSize range)
{
  descriptors->push_back(std::make_unique<Descriptor>(context, descriptorPool,
                                                      dynamic ? vk::DescriptorType::eUniformBufferDynamic :
                                                                vk::DescriptorType::eUniformBuffer,
                                                      shaderStageFlags, buffer->getBuffer(), range));
}