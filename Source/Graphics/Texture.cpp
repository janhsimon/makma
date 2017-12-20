#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

std::vector<std::shared_ptr<Texture>> Texture::textures;

vk::Buffer *Texture::createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage)
{
	auto bufferCreateInfo = vk::BufferCreateInfo().setSize(size).setUsage(usage);
	auto buffer = context->getDevice()->createBuffer(bufferCreateInfo);
	return new vk::Buffer(buffer);
}

vk::DeviceMemory *Texture::createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto memoryRequirements = context->getDevice()->getBufferMemoryRequirements(*buffer);
	auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	bool foundMatch = false;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
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

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	context->getDevice()->bindBufferMemory(*buffer, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);
}

vk::Image *Texture::createImage(const std::shared_ptr<Context> context, uint32_t width, uint32_t height, vk::Format format)
{
	auto imageCreateInfo = vk::ImageCreateInfo().setImageType(vk::ImageType::e2D).setExtent(vk::Extent3D(width, height, 1)).setMipLevels(1).setArrayLayers(1);
	imageCreateInfo.setFormat(format).setInitialLayout(vk::ImageLayout::ePreinitialized).setUsage(vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);
	auto image = context->getDevice()->createImage(imageCreateInfo);
	return new vk::Image(image);
}

vk::DeviceMemory *Texture::createImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags)
{
	auto memoryRequirements = context->getDevice()->getImageMemoryRequirements(*image);
	auto memoryProperties = context->getPhysicalDevice()->getMemoryProperties();

	uint32_t memoryTypeIndex = 0;
	bool foundMatch = false;
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
	{
		if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlags) == memoryPropertyFlags)
		{
			memoryTypeIndex = i;
			foundMatch = true;
			break;
		}
	}

	if (!foundMatch)
	{
		throw std::runtime_error("Failed to find suitable memory type for image.");
	}

	auto memoryAllocateInfo = vk::MemoryAllocateInfo().setAllocationSize(memoryRequirements.size).setMemoryTypeIndex(memoryTypeIndex);
	auto deviceMemory = context->getDevice()->allocateMemory(memoryAllocateInfo);
	context->getDevice()->bindImageMemory(*image, deviceMemory, 0);
	return new vk::DeviceMemory(deviceMemory);
}

vk::ImageView *Texture::createImageView(const std::shared_ptr<Context> context, const vk::Image *image, vk::Format format)
{
	auto imageViewCreateInfo = vk::ImageViewCreateInfo().setImage(*image).setViewType(vk::ImageViewType::e2D).setFormat(format);
	imageViewCreateInfo.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	auto imageView = context->getDevice()->createImageView(imageViewCreateInfo);
	return new vk::ImageView(imageView);
}

vk::Sampler *Texture::createSampler(const std::shared_ptr<Context> context)
{
	auto samplerCreateInfo = vk::SamplerCreateInfo().setMagFilter(vk::Filter::eLinear).setMinFilter(vk::Filter::eLinear).setMipmapMode(vk::SamplerMipmapMode::eLinear);
	samplerCreateInfo.setAnisotropyEnable(true).setMaxAnisotropy(16.0f);
	auto sampler = context->getDevice()->createSampler(samplerCreateInfo);
	return new vk::Sampler(sampler);
}

Texture::Texture(const std::shared_ptr<Context> context, const std::string &filename, vk::Format format)
{
	this->context = context;
	this->filename = filename;

	int pixelFormat = 0;
	int numChannels = 0;
	if (format == vk::Format::eR8G8B8A8Unorm)
	{
		pixelFormat = STBI_rgb_alpha;
		numChannels = 4;
	}
	else if (format == vk::Format::eR8Unorm)
	{
		pixelFormat = STBI_grey;
		numChannels = 1;
	}
	else
	{
		throw std::runtime_error("Failed to load texture from file \"" + filename + "\": Invalid pixel format.");
	}

	int width, height;
	stbi_uc *pixels = stbi_load(filename.c_str(), &width, &height, nullptr, pixelFormat);

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture from file \"" + filename + "\": Unable to load pixel data.");
	}

	VkDeviceSize imageSize = width * height * numChannels;

	std::unique_ptr<vk::Buffer, decltype(bufferDeleter)> stagingBuffer;
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> stagingBufferMemory;

	stagingBuffer = std::unique_ptr<vk::Buffer, decltype(bufferDeleter)>(createBuffer(context, imageSize, vk::BufferUsageFlagBits::eTransferSrc), bufferDeleter);
	stagingBufferMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createBufferMemory(context, stagingBuffer.get(), imageSize, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent), bufferMemoryDeleter);

	auto memory = context->getDevice()->mapMemory(*stagingBufferMemory, 0, imageSize);
	memcpy(memory, pixels, static_cast<size_t>(imageSize));
	context->getDevice()->unmapMemory(*stagingBufferMemory);

	image = std::unique_ptr<vk::Image, decltype(imageDeleter)>(createImage(context, width, height, format), imageDeleter);
	imageMemory = std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)>(createImageMemory(context, image.get(), vk::MemoryPropertyFlagBits::eDeviceLocal), bufferMemoryDeleter);

	auto commandBufferAllocateInfo = vk::CommandBufferAllocateInfo().setCommandPool(*context->getCommandPool()).setCommandBufferCount(1);
	auto commandBuffer = context->getDevice()->allocateCommandBuffers(commandBufferAllocateInfo).at(0);
	auto commandBufferBeginInfo = vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
	commandBuffer.begin(commandBufferBeginInfo);
	
	auto barrier = vk::ImageMemoryBarrier().setOldLayout(vk::ImageLayout::ePreinitialized).setNewLayout(vk::ImageLayout::eTransferDstOptimal).setImage(*image);
	barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1)).setDstAccessMask(vk::AccessFlagBits::eTransferWrite);
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	auto region = vk::BufferImageCopy().setImageExtent(vk::Extent3D(width, height, 1)).setImageSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));
	commandBuffer.copyBufferToImage(*stagingBuffer, *image, vk::ImageLayout::eTransferDstOptimal, 1, &region);
	
	barrier = vk::ImageMemoryBarrier().setOldLayout(vk::ImageLayout::eTransferDstOptimal).setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal).setImage(*image);
	barrier.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
	barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite).setDstAccessMask(vk::AccessFlagBits::eShaderRead);
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlags(), 0, nullptr, 0, nullptr, 1, &barrier);

	commandBuffer.end();
	auto submitInfo = vk::SubmitInfo().setCommandBufferCount(1).setPCommandBuffers(&commandBuffer);
	context->getQueue().submit({ submitInfo }, nullptr);
	context->getQueue().waitIdle();
	context->getDevice()->freeCommandBuffers(*context->getCommandPool(), 1, &commandBuffer);

	imageView = std::unique_ptr<vk::ImageView, decltype(imageViewDeleter)>(createImageView(context, image.get(), format), imageViewDeleter);
	sampler = std::unique_ptr<vk::Sampler, decltype(samplerDeleter)>(createSampler(context), samplerDeleter);
}

std::shared_ptr<Texture> Texture::cacheTexture(const std::shared_ptr<Context> context, const std::string &filename, vk::Format format)
{
	for (auto &texture : textures)
	{
		if (texture->filename.compare(filename) == 0)
		{
			return texture;
		}
	}

	auto texture = std::make_shared<Texture>(context, filename, format);
	textures.push_back(texture);
	return texture;
}