#include "Renderer.hpp"

#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // bring the depth range from [-1,1] (OpenGL) to [0,1] (Vulkan)
#include <gtc\matrix_transform.hpp>

Renderer::Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Camera> camera)
{
	this->window = window;
	this->camera = camera;

	context = std::make_shared<Context>(window);
	swapchain = std::make_unique<Swapchain>(window, context);
	
	buffers = std::make_shared<Buffers>(context);

	models = std::make_shared<std::vector<Model*>>();
	models->push_back(new Model(context, buffers, "Models\\Sponza\\Sponza.obj"));
	models->push_back(new Model(context, buffers, "Models\\OldMan\\OldMan.obj"));
	models->push_back(new Model(context, buffers, "Models\\Pistol\\sig.obj"));
	buffers->finalize();

	uint32_t materialCount = 0;
	for (auto model : *models.get())
	{
		materialCount += static_cast<uint32_t>(model->getMeshes()->size());
	}

	descriptor = std::make_shared<Descriptor>(context, materialCount);

	for (auto model : *models.get())
	{
		model->loadMaterials(descriptor);
	}

	pipeline = std::make_shared<Pipeline>(window, context, buffers, descriptor);
	
	swapchain->createFramebuffers(pipeline);
	swapchain->createCommandBuffers();

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	swapchain->recordCommandBuffers(pipeline, buffers, models, camera);
#endif

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::update(float delta)
{
	models->at(1)->setYaw(models->at(1)->getYaw() + delta * 0.1f);
	
	models->at(2)->setYaw(camera->getYaw());
	models->at(2)->setPitch(camera->getPitch() - 90.0f);
	models->at(2)->setRoll(camera->getRoll());
	models->at(2)->position = camera->position + camera->getForward() * -20.0f + camera->getUp() * -14.0f + camera->getRight() * 3.0f;
}

void Renderer::render()
{
#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	swapchain->recordCommandBuffers(pipeline, buffers, models, camera);
#endif

	auto nextImage = context->getDevice()->acquireNextImageKHR(*swapchain->getSwapchain(), std::numeric_limits<uint64_t>::max(), *semaphores->getImageAvailableSemaphore(), nullptr);

	if (nextImage.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to acquire next image for rendering.");
	}

	auto imageIndex = nextImage.value;
	auto stageFlags = vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput);
	auto submitInfo = vk::SubmitInfo().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getImageAvailableSemaphore()).setPWaitDstStageMask(&stageFlags);
	submitInfo.setCommandBufferCount(1).setPCommandBuffers(swapchain->getCommandBuffer(imageIndex)).setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getRenderFinishedSemaphore());
	context->getQueue().submit({ submitInfo }, nullptr);

	auto presentInfo = vk::PresentInfoKHR().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getRenderFinishedSemaphore());
	presentInfo.setSwapchainCount(1).setPSwapchains(swapchain->getSwapchain()).setPImageIndices(&imageIndex);
	if (context->getQueue().presentKHR(presentInfo) != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present queue.");
	}

	waitForIdle();
}

void Renderer::waitForIdle()
{
	context->getQueue().waitIdle();
}