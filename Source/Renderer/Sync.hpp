#pragma once

#include "Context.hpp"

class Sync
{
private:
	static const uint32_t MAX_FRAMES_IN_FLIGHT;
	
	std::shared_ptr<Context> context;
	uint32_t currentFrame;

	static vk::Semaphore *createSemaphore(const std::shared_ptr<Context> context);
	std::function<void(vk::Semaphore*)> semaphoreDeleter = [this](vk::Semaphore *semaphore) { if (context->getDevice()) context->getDevice()->destroySemaphore(*semaphore); };
	std::unique_ptr<vk::Semaphore, decltype(semaphoreDeleter)> shadowPassDoneSemaphore, geometryPassDoneSemaphore, lightingPassDoneSemaphore;

	static std::vector<vk::Semaphore> *createSemaphores(const std::shared_ptr<Context> context);
	std::function<void(std::vector<vk::Semaphore>*)> semaphoresDeleter = [this](std::vector<vk::Semaphore> *semaphores) { if (context->getDevice()) { for (auto &semaphore : *semaphores) context->getDevice()->destroySemaphore(semaphore); } };
	std::unique_ptr<std::vector<vk::Semaphore>, decltype(semaphoresDeleter)> imageAvailableSemaphores, compositePassDoneSemaphores;

	/*
	static std::vector<vk::Fence> *createFences(const std::shared_ptr<Context> context);
	std::function<void(std::vector<vk::Fence>*)> fencesDeleter = [this](std::vector<vk::Fence> *fences) { if (context->getDevice()) { for (auto &fence : *fences) context->getDevice()->destroyFence(fence); } };
	std::unique_ptr<std::vector<vk::Fence>, decltype(fencesDeleter)> fences;
	*/

public:
	Sync(const std::shared_ptr<Context> context);

	//void waitForFences() const;
	//void waitForAllFences() const;
	void advanceFrameIndex() { currentFrame = (++currentFrame) % MAX_FRAMES_IN_FLIGHT; }

	vk::Semaphore *getImageAvailableSemaphore() const { return &imageAvailableSemaphores.get()->at(currentFrame); }
	vk::Semaphore *getShadowPassDoneSemaphore() const { return shadowPassDoneSemaphore.get(); }
	vk::Semaphore *getGeometryPassDoneSemaphore() const { return geometryPassDoneSemaphore.get(); }
	vk::Semaphore *getLightingPassDoneSemaphore() const { return lightingPassDoneSemaphore.get(); }
	vk::Semaphore *getCompositePassDoneSemaphore() const { return &compositePassDoneSemaphores.get()->at(currentFrame); }
	//vk::Fence *getFence() const { return &fences.get()->at(currentFrame); }
};