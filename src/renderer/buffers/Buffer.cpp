#include "Buffer.hpp"

vk::Buffer*
Buffer::createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
  auto bufferCreateInfo = vk::BufferCreateInfo().setSize(size).setUsage(usage);
  auto buffer = context->getDevice()->createBuffer(bufferCreateInfo);
  return new vk::Buffer(buffer);
}

vk::DeviceMemory* Buffer::createMemory(const std::shared_ptr<Context> context,
                                       const vk::Buffer* buffer,
                                       vk::DeviceSize size,
                                       vk::MemoryPropertyFlags memoryPropertyFlags)
{
  auto memoryRequirements = context->getDevice()->getBufferMemoryRequirements(*buffer);
  auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

  uint32_t memoryTypeIndex = 0;
  bool foundMatch = false;
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
  {
    if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
        (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
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

  auto memoryAllocateInfo =
    vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
  auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
  context->getDevice()->bindBufferMemory(*buffer, deviceMemory, 0);
  return new vk::DeviceMemory(deviceMemory);
}

Buffer::Buffer(const std::shared_ptr<Context> context,
               vk::BufferUsageFlags usage,
               vk::DeviceSize size,
               vk::MemoryPropertyFlags memoryPropertyFlags)
{
  this->context = context;

  buffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, size, usage), bufferDeleter);
  memory = std::unique_ptr<vk::DeviceMemory, decltype(memoryDeleter)>(createMemory(context, buffer.get(), size,
                                                                                   memoryPropertyFlags),
                                                                      memoryDeleter);

  isMemoryMapped = false;
  mappedMemoryLocation = nullptr;
}

void Buffer::mapMemory()
{
  if (!isMemoryMapped)
  {
    mappedMemoryLocation = context->getDevice()->mapMemory(*memory, 0, VK_WHOLE_SIZE);
    isMemoryMapped = true;
  }
}

void Buffer::unmapMemory()
{
  if (isMemoryMapped)
  {
    context->getDevice()->unmapMemory(*memory);
    isMemoryMapped = false;
  }
}