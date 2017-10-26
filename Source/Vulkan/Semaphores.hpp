#pragma once

#include "Context.hpp"

class Semaphores
{
private:
	std::shared_ptr<Context> context;

	static vk::Semaphore *createSemaphore(const std::shared_ptr<Context> context);
	std::function<void(vk::Semaphore*)> semaphoreDeleter = [this](vk::Semaphore *semaphore) { if (context->getDevice()) context->getDevice()->destroySemaphore(*semaphore); };
	std::unique_ptr<vk::Semaphore, decltype(semaphoreDeleter)> imageAvailableSemaphore, renderFinishedSemaphore;

public:
	Semaphores(const std::shared_ptr<Context> context);

	vk::Semaphore *getImageAvailableSemaphore() const { return imageAvailableSemaphore.get(); }
	vk::Semaphore *getRenderFinishedSemaphore() const { return renderFinishedSemaphore.get(); }
};