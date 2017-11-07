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
	model = std::make_unique<Model>("Models\\Sponza\\Sponza.obj", buffers);
	buffers->finalize();

	texture = std::make_shared<Texture>("Textures\\Chalet.jpg", context);

	pipeline = std::make_shared<Pipeline>(window, context, buffers, texture);
	
	swapchain->createFramebuffers(pipeline);
	swapchain->createCommandBuffers();

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	swapchain->recordCommandBuffers(pipeline, buffers);
#endif

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::render()
{
#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	buffers->getPushConstants()->at(0) = glm::mat4(1.0f);
	buffers->getPushConstants()->at(1) = *camera->getViewMatrix();
	buffers->getPushConstants()->at(2) = *camera->getProjectionMatrix();
	buffers->getPushConstants()->at(2)[1][1] *= -1.0f;

	// we need to re-record the command buffers every frame for push constants to update
	swapchain->recordCommandBuffers(pipeline, buffers);
#else
	buffers->getUniformBufferObject()->worldMatrix = glm::mat4(1.0f);
	buffers->getUniformBufferObject()->viewMatrix = *camera->getViewMatrix();
	buffers->getUniformBufferObject()->projectionMatrix = *camera->getProjectionMatrix();
	buffers->getUniformBufferObject()->projectionMatrix[1][1] *= -1.0f;
	
	auto memory = context->getDevice()->mapMemory(*buffers->getUniformBufferMemory(), 0, sizeof(UniformBufferObject));
	memcpy(memory, buffers->getUniformBufferObject(), sizeof(UniformBufferObject));
	context->getDevice()->unmapMemory(*buffers->getUniformBufferMemory());
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