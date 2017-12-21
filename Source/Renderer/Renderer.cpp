#include "Renderer.hpp"

#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // bring the depth range from [-1,1] (OpenGL) to [0,1] (Vulkan)
#include <gtc\matrix_transform.hpp>

Renderer::Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera)
{
	this->window = window;
	this->input = input;
	this->camera = camera;

	context = std::make_shared<Context>(window);
	buffers = std::make_shared<Buffers>(context);

	Material::loadDefaultTextures(context);
}

std::shared_ptr<Model> Renderer::loadModel(const std::string &path, const std::string &filename)
{
	auto model = std::make_shared<Model>(context, buffers, path, filename);
	modelList.push_back(model);
	return model;
}

std::shared_ptr<Light> Renderer::loadLight(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, float specularPower)
{
	auto light = std::make_shared<Light>(type, position, color, range, intensity, specularPower);
	lightList.push_back(light);
	return light;
}

void Renderer::finalize()
{
	buffers->finalize(static_cast<uint32_t>(modelList.size()), static_cast<uint32_t>(lightList.size()));

	descriptor = std::make_shared<Descriptor>(context, buffers, Material::getNumMaterials());
	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptor);
	}

	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptor);
	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, buffers, descriptor, geometryBuffer->getRenderPass());

	swapchain = std::make_unique<Swapchain>(window, context);
	lightingPipeline = std::make_shared<LightingPipeline>(window, context, descriptor, swapchain->getRenderPass());
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptor, buffers, &lightList);

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	geometryBuffer->recordCommandBuffer(geometryPipeline, buffers, &modelList);

	buffers->getViewProjectionData()->projectionMatrix = *camera.get()->getProjectionMatrix();
	buffers->getViewProjectionData()->projectionMatrix[1][1] *= -1.0f;
#endif

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::update()
{
#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS

	// world matrix

	auto memory = context->getDevice()->mapMemory(*buffers->getWorldUniformBufferMemory(), 0, sizeof(WorldData));
	for (size_t i = 0; i < modelList.size(); ++i)
	{
		auto dst = ((char*)memory) + i * buffers->getWorldDataAlignment();
		memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getWorldUniformBufferMemory()).setSize(sizeof(WorldData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getWorldUniformBufferMemory());


	// light data

	memory = context->getDevice()->mapMemory(*buffers->getLightUniformBufferMemory(), 0, sizeof(LightData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		auto light = lightList.at(i);

		glm::mat4 lightData;
		lightData[0] = glm::vec4(light->position, light->type == LightType::Directional ? 0.0f : 1.0f);
		lightData[1] = glm::vec4(light->color, light->intensity);
		lightData[2] = glm::vec4(light->range, light->specularPower, 0.0f, 0.0f);
		lightData[3] = glm::vec4(0.0f);
		
		auto dst = ((char*)memory) + i * buffers->getLightDataAlignment();
		memcpy(dst, &lightData, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightUniformBufferMemory()).setSize(sizeof(LightData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightUniformBufferMemory());


	// view and projection matrices

	buffers->getViewProjectionData()->viewMatrix = *camera.get()->getViewMatrix();
	memory = context->getDevice()->mapMemory(*buffers->getViewProjectionUniformBufferMemory(), 0, sizeof(ViewProjectionData));
	memcpy(memory, buffers->getViewProjectionData(), sizeof(ViewProjectionData));
	context->getDevice()->unmapMemory(*buffers->getViewProjectionUniformBufferMemory());


	// eye position data

	buffers->getEyePositionData()->eyePosition = camera.get()->position;
	memory = context->getDevice()->mapMemory(*buffers->getEyePositionUniformBufferMemory(), 0, sizeof(EyePositionData));
	memcpy(memory, buffers->getEyePositionData(), sizeof(EyePositionData));
	context->getDevice()->unmapMemory(*buffers->getEyePositionUniformBufferMemory());

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