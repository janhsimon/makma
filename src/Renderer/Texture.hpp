#pragma once

#include "Context.hpp"

class Texture
{
private:
	std::shared_ptr<Context> context;

	std::string filename;
	uint32_t mipLevels;

	static vk::Buffer *createBuffer(const std::shared_ptr<Context> context, vk::DeviceSize size, vk::BufferUsageFlags usage);
	std::function<void(vk::Buffer*)> bufferDeleter = [this](vk::Buffer *buffer) { if (context->getDevice()) context->getDevice()->destroyBuffer(*buffer); };

	static vk::DeviceMemory *createBufferMemory(const std::shared_ptr<Context> context, const vk::Buffer *buffer, vk::DeviceSize size, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::function<void(vk::DeviceMemory*)> bufferMemoryDeleter = [this](vk::DeviceMemory *bufferMemory) { if (context->getDevice()) context->getDevice()->freeMemory(*bufferMemory); };

	static vk::Image *createImage(const std::shared_ptr<Context> context, uint32_t width, uint32_t height, vk::Format format, uint32_t mipLevels);
	std::function<void(vk::Image*)> imageDeleter = [this](vk::Image *image) { if (context->getDevice()) context->getDevice()->destroyImage(*image); };
	std::unique_ptr<vk::Image, decltype(imageDeleter)> image;

	static vk::DeviceMemory *createImageMemory(const std::shared_ptr<Context> context, const vk::Image *image, vk::MemoryPropertyFlags memoryPropertyFlags);
	std::unique_ptr<vk::DeviceMemory, decltype(bufferMemoryDeleter)> imageMemory;

	static vk::ImageView *createImageView(const std::shared_ptr<Context> context, const vk::Image *image, vk::Format format, uint32_t mipLevels);
	std::function<void(vk::ImageView*)> imageViewDeleter = [this](vk::ImageView *imageView) { if (context->getDevice()) context->getDevice()->destroyImageView(*imageView); };
	std::unique_ptr<vk::ImageView, decltype(imageViewDeleter)> imageView;

	static vk::Sampler *createSampler(const std::shared_ptr<Context> context, uint32_t mipLevels);
	std::function<void(vk::Sampler*)> samplerDeleter = [this](vk::Sampler *sampler) { if (context->getDevice()) context->getDevice()->destroySampler(*sampler); };
	std::unique_ptr<vk::Sampler, decltype(samplerDeleter)> sampler;

	static std::vector<std::shared_ptr<Texture>> textures;

public:
	Texture(const std::shared_ptr<Context> context, const std::string &filename, vk::Format format);

	vk::ImageView *getImageView() const { return imageView.get(); }
	vk::Sampler *getSampler() const { return sampler.get(); }

	static std::shared_ptr<Texture> cacheTexture(const std::shared_ptr<Context> context, const std::string &filename, vk::Format format);
};
