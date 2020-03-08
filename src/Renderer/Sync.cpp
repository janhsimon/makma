#include "Sync.hpp"

const uint32_t Sync::MAX_FRAMES_IN_FLIGHT = 2;

vk::Semaphore *Sync::createSemaphore(const std::shared_ptr<Context> context)
{
	auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
	auto semaphore = context->getDevice()->createSemaphore(semaphoreCreateInfo);
	return new vk::Semaphore(semaphore);
}

std::vector<vk::Semaphore> *Sync::createSemaphores(const std::shared_ptr<Context> context)
{
	auto semaphores = std::vector<vk::Semaphore>(MAX_FRAMES_IN_FLIGHT);
	auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
	for (size_t i = 0; i < semaphores.size(); ++i)
	{
		semaphores[i] = context->getDevice()->createSemaphore(semaphoreCreateInfo);
	}

	return new std::vector<vk::Semaphore>(semaphores);
}

/*
std::vector<vk::Fence> *Sync::createFences(const std::shared_ptr<Context> context)
{
	auto fences = std::vector<vk::Fence>(MAX_FRAMES_IN_FLIGHT);
	auto fenceCreateInfo = vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled);
	for (size_t i = 0; i < fences.size(); ++i)
	{
		fences[i] = context->getDevice()->createFence(fenceCreateInfo);
	}

	return new std::vector<vk::Fence>(fences);
}
*/

Sync::Sync(const std::shared_ptr<Context> context)
{
	this->context = context;

	shadowPassDoneSemaphore = std::unique_ptr<vk::Semaphore, decltype(semaphoreDeleter)>(createSemaphore(context), semaphoreDeleter);
	geometryPassDoneSemaphore = std::unique_ptr<vk::Semaphore, decltype(semaphoreDeleter)>(createSemaphore(context), semaphoreDeleter);
	lightingPassDoneSemaphore = std::unique_ptr<vk::Semaphore, decltype(semaphoreDeleter)>(createSemaphore(context), semaphoreDeleter);
	
	imageAvailableSemaphores = std::unique_ptr<std::vector<vk::Semaphore>, decltype(semaphoresDeleter)>(createSemaphores(context), semaphoresDeleter);
	compositePassDoneSemaphores = std::unique_ptr<std::vector<vk::Semaphore>, decltype(semaphoresDeleter)>(createSemaphores(context), semaphoresDeleter);

	//fences = std::unique_ptr<std::vector<vk::Fence>, decltype(fencesDeleter)>(createFences(context), fencesDeleter);

	currentFrame = 0;
}

/*
void Sync::waitForFences() const
{
	context->getDevice()->waitForFences(1, &fences->at(currentFrame), true, std::numeric_limits<uint64_t>::max());
	context->getDevice()->resetFences(1, &fences->at(currentFrame));
}

void Sync::waitForAllFences() const
{
	context->getDevice()->waitForFences(static_cast<uint32_t>(fences->size()), fences->data(), true, std::numeric_limits<uint64_t>::max());
	context->getDevice()->resetFences(static_cast<uint32_t>(fences->size()), fences->data());
}
*/