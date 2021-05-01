#include "Descriptor.hpp"

vk::DescriptorSetLayout* Descriptor::createLayout(const std::shared_ptr<Context> context,
                                                  vk::DescriptorType type,
                                                  vk::ShaderStageFlags shaderStageFlags)
{
  auto layoutBinding =
    vk::DescriptorSetLayoutBinding().setDescriptorCount(1).setDescriptorType(type).setStageFlags(shaderStageFlags);
  auto descriptorSetLayoutCreateInfo =
    vk::DescriptorSetLayoutCreateInfo().setBindingCount(1).setPBindings(&layoutBinding);
  return new vk::DescriptorSetLayout(context->getDevice()->createDescriptorSetLayout(descriptorSetLayoutCreateInfo));
}

vk::DescriptorSet* Descriptor::createSet(const std::shared_ptr<Context> context,
                                         const std::shared_ptr<DescriptorPool> descriptorPool,
                                         const vk::DescriptorSetLayout* layout,
                                         const vk::Buffer* buffer,
                                         vk::DeviceSize range,
                                         vk::DescriptorType type)
{
  auto descriptorSetAllocateInfo = vk::DescriptorSetAllocateInfo()
                                     .setDescriptorPool(*descriptorPool->getPool())
                                     .setDescriptorSetCount(1)
                                     .setPSetLayouts(layout);
  auto descriptorSet = context->getDevice()->allocateDescriptorSets(descriptorSetAllocateInfo).at(0);
  auto descriptorBufferInfo = vk::DescriptorBufferInfo().setBuffer(*buffer).setRange(range);
  auto writeDescriptorSet = vk::WriteDescriptorSet()
                              .setDstSet(descriptorSet)
                              .setDescriptorType(type)
                              .setDescriptorCount(1)
                              .setPBufferInfo(&descriptorBufferInfo);
  context->getDevice()->updateDescriptorSets(1, &writeDescriptorSet, 0, nullptr);
  return new vk::DescriptorSet(descriptorSet);
}

Descriptor::Descriptor(const std::shared_ptr<Context> context,
                       const std::shared_ptr<DescriptorPool> descriptorPool,
                       vk::DescriptorType type,
                       vk::ShaderStageFlags shaderStageFlags,
                       const vk::Buffer* buffer,
                       vk::DeviceSize range)
{
  this->context = context;
  this->descriptorPool = descriptorPool;

  layout =
    std::unique_ptr<vk::DescriptorSetLayout, decltype(layoutDeleter)>(createLayout(context, type, shaderStageFlags),
                                                                      layoutDeleter);
  set = std::unique_ptr<vk::DescriptorSet>(createSet(context, descriptorPool, layout.get(), buffer, range, type));
}
