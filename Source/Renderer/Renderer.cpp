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

	unitQuadModel = loadModel("Models\\UnitQuad\\", "UnitQuad.obj");
	unitSphereModel = loadModel("Models\\UnitSphere\\", "UnitSphere.obj");
}

std::shared_ptr<Model> Renderer::loadModel(const std::string &path, const std::string &filename)
{
	auto model = std::make_shared<Model>(context, buffers, path, filename);
	modelList.push_back(model);
	return model;
}

std::shared_ptr<Light> Renderer::loadLight(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, float specularPower, bool castShadows)
{
	auto light = std::make_shared<Light>(type, position, color, range, intensity, specularPower, castShadows);
	lightList.push_back(light);
	return light;
}

void Renderer::finalize()
{
	buffers->finalize(static_cast<uint32_t>(modelList.size()), static_cast<uint32_t>(lightList.size()));

	uint32_t numShadowMaps = 0;
	for (auto &light : lightList)
	{
		if (light->castShadows)
		{
			++numShadowMaps;
		}
	}

	descriptor = std::make_shared<Descriptor>(context, buffers, Material::getNumMaterials(), numShadowMaps);

	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptor);
	}

	shadowPipeline = std::make_shared<ShadowPipeline>(context, buffers, descriptor);

	for (uint32_t i = 0; i < lightList.size(); ++i)
	{
		lightList[i]->finalize(context, buffers, descriptor, shadowPipeline, &modelList, i);
	}

	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptor);
	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, buffers, descriptor, geometryBuffer->getRenderPass());

	swapchain = std::make_unique<Swapchain>(window, context, unitQuadModel, unitSphereModel);
	lightingPipeline = std::make_shared<LightingPipeline>(window, context, descriptor, swapchain->getRenderPass());
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptor, buffers, &lightList);

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	geometryBuffer->recordCommandBuffer(geometryPipeline, buffers, &modelList);
#endif

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::update()
{
#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS

	// shadow pass vertex dynamic uniform buffer

	auto memory = context->getDevice()->mapMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory(), 0, sizeof(ShadowPassVertexDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		auto light = lightList.at(i);

		// TODO: replace this code, put matrix into light
		auto shadowMapViewMatrix = glm::lookAt(light->position * -4000.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		auto shadowMapProjectionMatrix = glm::ortho(-2500.0f, 2500.0f, -2500.0f, 2500.0f, 0.0f, 5000.0f);
		shadowMapProjectionMatrix[1][1] *= -1.0f;
		auto lightMatrix = shadowMapProjectionMatrix * shadowMapViewMatrix;

		auto dst = ((char*)memory) + i * buffers->getSingleMat4DataAlignment();
		memcpy(dst, &lightMatrix, sizeof(glm::mat4));
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory()).setSize(sizeof(ShadowPassVertexDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory());


	// geometry pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getGeometryPassVertexDynamicUniformBufferMemory(), 0, sizeof(GeometryPassVertexDynamicData));
	for (size_t i = 0; i < modelList.size(); ++i)
	{
		auto dst = ((char*)memory) + i * buffers->getSingleMat4DataAlignment();
		memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getGeometryPassVertexDynamicUniformBufferMemory()).setSize(sizeof(GeometryPassVertexDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getGeometryPassVertexDynamicUniformBufferMemory());


	// geometry pass vertex uniform buffer

	buffers->getGeometryPassVertexData()->viewMatrix = *camera.get()->getViewMatrix();
	buffers->getGeometryPassVertexData()->projectionMatrix = *camera.get()->getProjectionMatrix();
	buffers->getGeometryPassVertexData()->projectionMatrix[1][1] *= -1.0f;
	memory = context->getDevice()->mapMemory(*buffers->getGeometryPassVertexUniformBufferMemory(), 0, sizeof(GeometryPassVertexData));
	memcpy(memory, buffers->getGeometryPassVertexData(), sizeof(GeometryPassVertexData));
	context->getDevice()->unmapMemory(*buffers->getGeometryPassVertexUniformBufferMemory());


	// lighting pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory(), 0, sizeof(LightingPassVertexDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		auto light = lightList.at(i);

		auto lightMatrix = glm::mat4(1.0f);
		if (lightList[i]->type == LightType::Point)
		{
			glm::mat4 worldMatrix;
			worldMatrix = glm::translate(worldMatrix, glm::vec3(light->position[0], light->position[1], light->position[2]));
			worldMatrix = glm::scale(worldMatrix, glm::vec3(light->range));

			glm::mat4 cameraProjectionMatrix = *camera->getProjectionMatrix();
			cameraProjectionMatrix[1][1] *= -1.0f;
			lightMatrix = cameraProjectionMatrix * (*camera->getViewMatrix()) * worldMatrix;
		}

		auto dst = ((char*)memory) + i * buffers->getSingleMat4DataAlignment();
		memcpy(dst, &lightMatrix, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory()).setSize(sizeof(LightingPassVertexDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory());


	// lighting pass fragment dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory(), 0, sizeof(LightingPassFragmentDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		auto light = lightList.at(i);

		glm::mat4 lightData;
		lightData[0] = glm::vec4(light->position, light->type == LightType::Directional ? 0.0f : 1.0f);
		lightData[1] = glm::vec4(light->color, light->intensity);
		lightData[2] = glm::vec4(light->range, light->specularPower, light->castShadows ? 1.0f: 0.0f, 0.0f);
		lightData[3] = glm::vec4(0.0f);

		// TODO: replace this code, put matrix into light
		auto shadowMapViewMatrix = glm::lookAt(light->position * -4000.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		auto shadowMapProjectionMatrix = glm::ortho(-2500.0f, 2500.0f, -2500.0f, 2500.0f, 0.0f, 5000.0f);
		shadowMapProjectionMatrix[1][1] *= -1.0f;
		auto lightMatrix = shadowMapProjectionMatrix * shadowMapViewMatrix;
		
		auto dst = ((char*)memory) + i * buffers->getDoubleMat4DataAlignment();
		memcpy(dst, &lightData, sizeof(glm::mat4));
		dst += sizeof(glm::mat4);
		memcpy(dst, &lightMatrix, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory()).setSize(sizeof(LightingPassFragmentDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory());


	// lighting pass fragment uniform buffer

	buffers->getLightingPassFragmentData()->data[0] = glm::vec4(camera.get()->position, 0.0f);
	buffers->getLightingPassFragmentData()->data[1] = glm::vec4(window->getWidth(), window->getHeight(), 0.0f, 0.0f);
	buffers->getLightingPassFragmentData()->data[2] = glm::vec4(0.0f);
	buffers->getLightingPassFragmentData()->data[3] = glm::vec4(0.0f);
	memory = context->getDevice()->mapMemory(*buffers->getLightingPassFragmentUniformBufferMemory(), 0, sizeof(LightingPassFragmentData));
	memcpy(memory, buffers->getLightingPassFragmentData(), sizeof(LightingPassFragmentData));
	context->getDevice()->unmapMemory(*buffers->getLightingPassFragmentUniformBufferMemory());

#endif
}

void Renderer::render()
{
	// shadow pass

	auto submitInfo = vk::SubmitInfo().setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getShadowPassDoneSemaphore());
	std::vector<vk::CommandBuffer> commandBuffers;

	for (auto &light : lightList)
	{
		if (light->castShadows)
		{
			commandBuffers.push_back(*light->getCommandBuffer());
		}
	}

	submitInfo.setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size())).setPCommandBuffers(commandBuffers.data());
	context->getQueue().submit({ submitInfo }, nullptr);


	// geometry pass

#ifdef MK_OPTIMIZATION_PUSH_CONSTANTS
	geometryBuffer->recordCommandBuffer(geometryPipeline, buffers, &models, camera);
#endif

	vk::PipelineStageFlags stageFlags2[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	submitInfo = vk::SubmitInfo().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getShadowPassDoneSemaphore()).setPWaitDstStageMask(stageFlags2);
	submitInfo.setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getGeometryPassDoneSemaphore()).setCommandBufferCount(1).setPCommandBuffers(geometryBuffer->getCommandBuffer());
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