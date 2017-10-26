#pragma once

#include "Vulkan.hpp"

bool createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usageFlags, vk::MemoryPropertyFlags memoryPropertyFlags, vk::Buffer &buffer, vk::DeviceMemory &deviceMemory);
bool copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);