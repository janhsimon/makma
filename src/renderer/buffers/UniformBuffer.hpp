#pragma once

#include "Buffer.hpp"
#include "Descriptor.hpp"

class UniformBuffer
{
private:
  std::unique_ptr<Buffer> buffer;
  std::unique_ptr<std::vector<std::unique_ptr<Descriptor>>> descriptors;
  std::shared_ptr<Context> context;
  bool dynamic;

public:
  UniformBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, bool dynamic);

  void addDescriptor(const std::shared_ptr<DescriptorPool> descriptorPool,
                     vk::ShaderStageFlagBits shaderStageFlags,
                     vk::DeviceSize range);

  Buffer* getBuffer() const
  {
    return buffer.get();
  }
  Descriptor* getDescriptor(const uint32_t index) const
  {
    return descriptors->at(index).get();
  }
};