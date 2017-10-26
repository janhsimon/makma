#include "Renderer.hpp"

void Renderer::create(const Window *window)
{
	context = std::make_shared<Context>(window);
	swapchain = std::make_unique<Swapchain>(window, context);
	pipeline = std::make_shared<Pipeline>(window, context);
	buffers = std::make_shared<Buffers>(context);
	semaphores = std::make_unique<Semaphores>(context);

	swapchain->finalize(window, pipeline, buffers);
}

void Renderer::render()
{
	auto nextImage = context->getDevice()->acquireNextImageKHR(*swapchain->getSwapchain(), std::numeric_limits<uint64_t>::max(), *semaphores->getImageAvailableSemaphore(), nullptr);

	if (nextImage.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to acquire next image for rendering.");
	}

	auto imageIndex = nextImage.value;
	auto stageFlags = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	auto submitInfo = vk::SubmitInfo().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getImageAvailableSemaphore()).setPWaitDstStageMask(&stageFlags);
	submitInfo.setCommandBufferCount(1).setPCommandBuffers(&swapchain->getCommandBuffer(imageIndex)).setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getRenderFinishedSemaphore());
	
	if (context->getQueue().submit(1, &submitInfo, nullptr) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to submit queue.");
	}

	auto presentInfo = vk::PresentInfoKHR().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getRenderFinishedSemaphore());
	presentInfo.setSwapchainCount(1).setPSwapchains(swapchain->getSwapchain()).setPImageIndices(&imageIndex);
	if (context->getQueue().presentKHR(presentInfo) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present queue.");
	}
}

void Renderer::waitForIdle()
{
	context->getQueue().waitIdle();
}