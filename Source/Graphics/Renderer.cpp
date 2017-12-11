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
	
	buffers = std::make_shared<Buffers>(context);
	models.push_back(new Model(context, buffers, "Models\\Sponza\\Sponza.fbx"));
	models.push_back(new Model(context, buffers, "Models\\OldMan\\OldMan.fbx"));
	models.push_back(new Model(context, buffers, "Models\\Machinegun\\Machinegun.fbx"));
	buffers->finalize(static_cast<uint32_t>(models.size()));

	descriptor = std::make_shared<Descriptor>(context, buffers, Material::getNumMaterials());

	for (auto &model : models)
	{
		model->finalizeMaterials(descriptor);
	}

	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptor);
	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, buffers, descriptor, geometryBuffer->getRenderPass());

	swapchain = std::make_unique<Swapchain>(window, context);
	lightingPipeline = std::make_shared<LightingPipeline>(window, context, descriptor, swapchain->getRenderPass());
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptor);

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	geometryBuffer->recordCommandBuffer(geometryPipeline, buffers, &models, camera);
#endif

	semaphores = std::make_unique<Semaphores>(context);
	
	buffers->getViewProjectionData()->projectionMatrix = *camera.get()->getProjectionMatrix();
	buffers->getViewProjectionData()->projectionMatrix[1][1] *= -1.0f;

	buffers->getLightData()->directionalLightsDirection[0] = glm::vec4(0.0f, -0.5f, -1.0f, 0.0f);
	buffers->getLightData()->directionalLightsColor[0] = glm::vec4(1.0f, 0.75f, 0.5f, 1.0f);

	buffers->getLightData()->directionalLightsDirection[1] = glm::vec4(0.0f, -0.5f, 1.0f, 0.0f);
	buffers->getLightData()->directionalLightsColor[1] = glm::vec4(0.5f, 0.75f, 1.0f, 1.0f);

	buffers->getLightData()->directionalLightsDirection[2] = glm::vec4(-1.0f, -0.5f, 0.0f, 0.0f);
	buffers->getLightData()->directionalLightsColor[2] = glm::vec4(1.0f, 0.75f, 0.5f, 1.0f);

	buffers->getLightData()->directionalLightsDirection[3] = glm::vec4(1.0f, -0.5f, 0.0f, 0.0f);
	buffers->getLightData()->directionalLightsColor[3] = glm::vec4(0.5f, 0.75f, 1.0f, 1.0f);
}

void Renderer::update(float delta)
{
	models.at(1)->setYaw(models.at(1)->getYaw() + delta * 0.1f);
	
	/*
	// values for the pistol
	models->at(2)->position = camera->position - camera->getForward() * 22.5f - camera->getUp() * 12.0f + camera->getRight() * 4.0f;
	models->at(2)->setYaw(camera->getYaw());
	models->at(2)->setPitch(camera->getPitch() - 90.0f);
	models->at(2)->setRoll(camera->getRoll());
	*/

	// values for the m249
	models.at(2)->position = camera->position + camera->getForward() * 150.0f - camera->getUp() * 75.0f - camera->getRight() * 40.0f;
	models.at(2)->setYaw(camera->getYaw());
	models.at(2)->setPitch(camera->getPitch() - 90.0f);
	models.at(2)->setRoll(camera->getRoll());

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	auto memory = context->getDevice()->mapMemory(*buffers->getDynamicUniformBufferMemory(), 0, sizeof(DynamicUniformBufferData));
	
	for (size_t i = 0; i < models.size(); ++i)
	{
		auto dst = ((char*)memory) + i * buffers->getDynamicAlignment();
		memcpy(dst, &models.at(i)->getWorldMatrix(), sizeof(glm::mat4));
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getDynamicUniformBufferMemory()).setSize(sizeof(DynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);

	context->getDevice()->unmapMemory(*buffers->getDynamicUniformBufferMemory());

	buffers->getViewProjectionData()->viewMatrix = *camera.get()->getViewMatrix();
	memory = context->getDevice()->mapMemory(*buffers->getViewProjectionUniformBufferMemory(), 0, sizeof(ViewProjectionData));
	memcpy(memory, buffers->getViewProjectionData(), sizeof(ViewProjectionData));
	context->getDevice()->unmapMemory(*buffers->getViewProjectionUniformBufferMemory());

	buffers->getLightData()->directionalLightsColor[0].r -= delta * 0.001f;
	buffers->getLightData()->directionalLightsColor[1].g -= delta * 0.001f;
	buffers->getLightData()->directionalLightsColor[2].b -= delta * 0.001f;
	if (buffers->getLightData()->directionalLightsColor[0].r < 0.0f) buffers->getLightData()->directionalLightsColor[0].r = 1.0f;
	if (buffers->getLightData()->directionalLightsColor[1].g < 0.0f) buffers->getLightData()->directionalLightsColor[1].g = 1.0f;
	if (buffers->getLightData()->directionalLightsColor[2].b < 0.0f) buffers->getLightData()->directionalLightsColor[2].b = 1.0f;
	memory = context->getDevice()->mapMemory(*buffers->getLightUniformBufferMemory(), 0, sizeof(LightData));
	memcpy(memory, buffers->getLightData(), sizeof(LightData));
	context->getDevice()->unmapMemory(*buffers->getLightUniformBufferMemory());
#endif
}

void Renderer::render()
{
	// geometry pass

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	geometryBuffer->recordCommandBuffer(geometryPipeline, buffers, &models, camera);
#endif

	auto submitInfo = vk::SubmitInfo().setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getGeometryPassDoneSemaphore());
	submitInfo.setCommandBufferCount(1).setPCommandBuffers(geometryBuffer->getCommandBuffer());
	context->getQueue().submit({ submitInfo }, nullptr);


	// lighting pass
	
	auto nextImage = context->getDevice()->acquireNextImageKHR(*swapchain->getSwapchain(), std::numeric_limits<uint64_t>::max(), *semaphores->getImageAvailableSemaphore(), nullptr);
	if (nextImage.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to acquire next image for rendering.");
	}

	auto imageIndex = nextImage.value;
	vk::PipelineStageFlags stageFlags[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::vector<vk::Semaphore> waitingForSemaphores = { *semaphores->getGeometryPassDoneSemaphore(), *semaphores->getImageAvailableSemaphore() };
	submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitingForSemaphores.size())).setPWaitSemaphores(waitingForSemaphores.data()).setPWaitDstStageMask(stageFlags);
	submitInfo.setPCommandBuffers(swapchain->getCommandBuffer(imageIndex)).setPSignalSemaphores(semaphores->getLightingPassDoneSemaphore());
	context->getQueue().submit({ submitInfo }, nullptr);


	// present

	auto presentInfo = vk::PresentInfoKHR().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getLightingPassDoneSemaphore());
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