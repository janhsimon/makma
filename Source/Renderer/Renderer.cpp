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

	unitQuadModel = std::make_shared<Model>(context, buffers, "Models\\UnitQuad\\", "UnitQuad.obj");
	unitSphereModel = std::make_shared<Model>(context, buffers, "Models\\UnitSphere\\", "UnitSphere.obj");
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
	uint32_t numShadowMaps = 0;
	for (auto &light : lightList)
	{
		if (light->castShadows)
		{
			++numShadowMaps;
		}
	}

	buffers->finalize(static_cast<uint32_t>(modelList.size()), static_cast<uint32_t>(lightList.size()), numShadowMaps);
	descriptor = std::make_shared<Descriptor>(context, buffers, Material::getNumMaterials(), numShadowMaps);

	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptor);
	}

	shadowPipeline = std::make_shared<ShadowPipeline>(context, buffers, descriptor);

	uint32_t shadowMapIndex = 0;
	for (uint32_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList[i];
		light->finalize(context, buffers, descriptor, shadowPipeline, &modelList, light->castShadows ? (shadowMapIndex++) : 0, numShadowMaps);
	}

	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptor);
	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, buffers, descriptor, geometryBuffer->getRenderPass());

	swapchain = std::make_unique<Swapchain>(window, context, unitQuadModel, unitSphereModel);
	lightingPipeline = std::make_shared<LightingPipeline>(window, context, descriptor, swapchain->getRenderPass());
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptor, buffers, &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()));

#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	geometryBuffer->recordCommandBuffer(geometryPipeline, buffers, &modelList, numShadowMaps);
#endif

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::update()
{
#ifndef MK_OPTIMIZATION_PUSH_CONSTANTS
#ifdef MK_OPTIMIZATION_GLOBAL_UNIFORM_BUFFERS

	// uniform buffer

	auto cameraProjectionMatrix = *camera->getProjectionMatrix();
	cameraProjectionMatrix[1][1] *= -1.0f;
	buffers->getUniformBufferData()->viewProjectionMatrix = cameraProjectionMatrix * (*camera->getViewMatrix());

	buffers->getUniformBufferData()->data[0] = glm::vec4(camera->position, 0.0f);
	buffers->getUniformBufferData()->data[1] = glm::vec4(window->getWidth(), window->getHeight(), 0.0f, 0.0f);

	auto memory = context->getDevice()->mapMemory(*buffers->getUniformBufferMemory(), 0, sizeof(UniformBufferData));
	memcpy(memory, buffers->getUniformBufferData(), sizeof(UniformBufferData));
	context->getDevice()->unmapMemory(*buffers->getUniformBufferMemory());


	// dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getDynamicUniformBufferMemory(), 0, sizeof(DynamicUniformBufferData));
	auto dst = ((char*)memory);
	
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		if (light->castShadows)
		{
			// TODO: replace this code, put matrix into light
			auto shadowMapViewMatrix = glm::lookAt(light->position * -5500.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
			auto shadowMapProjectionMatrix = glm::ortho(-2500.0f, 2500.0f, -2500.0f, 2500.0f, 0.0f, 7500.0f);
			shadowMapProjectionMatrix[1][1] *= -1.0f;
			auto lightMatrix = shadowMapProjectionMatrix * shadowMapViewMatrix;

			memcpy(dst, &lightMatrix, sizeof(glm::mat4));
			dst += buffers->getDataAlignment();
		}
	}

	for (size_t i = 0; i < modelList.size(); ++i)
	{
		memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
		dst += buffers->getDataAlignment();
	}

	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		auto lightMatrix = glm::mat4(1.0f);
		if (light->type == LightType::Point)
		{
			lightMatrix = cameraProjectionMatrix * (*camera->getViewMatrix()) * light->getWorldMatrix();
		}

		memcpy(dst, &lightMatrix, sizeof(glm::mat4));
		dst += buffers->getDataAlignment();
	}

	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		// TODO: replace this code, put data into light
		glm::mat4 lightData;
		lightData[0] = glm::vec4(light->position, light->type == LightType::Directional ? 0.0f : 1.0f);
		lightData[1] = glm::vec4(light->color, light->intensity);
		lightData[2] = glm::vec4(light->getRange(), light->specularPower, light->castShadows ? 1.0f : 0.0f, 0.0f);
		lightData[3] = glm::vec4(0.0f);

		memcpy(dst, &lightData, sizeof(glm::mat4));
		dst += buffers->getDataAlignment();
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getDynamicUniformBufferMemory()).setSize(sizeof(DynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getDynamicUniformBufferMemory());

#else

	// shadow pass vertex dynamic uniform buffer

	auto memory = context->getDevice()->mapMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory(), 0, sizeof(ShadowPassDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		if (!light->castShadows)
		{
			continue;
		}

		// TODO: replace this code, put matrix into light
		auto shadowMapViewMatrix = glm::lookAt(light->position * -5500.0f, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		auto shadowMapProjectionMatrix = glm::ortho(-2500.0f, 2500.0f, -2500.0f, 2500.0f, 0.0f, 7500.0f);
		shadowMapProjectionMatrix[1][1] *= -1.0f;
		auto lightMatrix = shadowMapProjectionMatrix * shadowMapViewMatrix;

		auto dst = ((char*)memory) + i * buffers->getDataAlignment();
		memcpy(dst, &lightMatrix, sizeof(glm::mat4));
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory()).setSize(sizeof(ShadowPassDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory());


	// geometry pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getGeometryPassVertexDynamicUniformBufferMemory(), 0, sizeof(GeometryPassVertexDynamicData));
	for (size_t i = 0; i < modelList.size(); ++i)
	{
		auto dst = ((char*)memory) + i * buffers->getDataAlignment();
		memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getGeometryPassVertexDynamicUniformBufferMemory()).setSize(sizeof(GeometryPassVertexDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getGeometryPassVertexDynamicUniformBufferMemory());


	// geometry pass vertex uniform buffer

	auto cameraProjectionMatrix = *camera.get()->getProjectionMatrix();
	cameraProjectionMatrix[1][1] *= -1.0f;
	buffers->getGeometryPassVertexData()->viewProjectionMatrix = cameraProjectionMatrix * (*camera.get()->getViewMatrix());
	memory = context->getDevice()->mapMemory(*buffers->getGeometryPassVertexUniformBufferMemory(), 0, sizeof(GeometryPassVertexData));
	memcpy(memory, buffers->getGeometryPassVertexData(), sizeof(GeometryPassVertexData));
	context->getDevice()->unmapMemory(*buffers->getGeometryPassVertexUniformBufferMemory());


	// lighting pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory(), 0, sizeof(LightingPassVertexDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		auto lightMatrix = glm::mat4(1.0f);
		if (lightList[i]->type == LightType::Point)
		{
			lightMatrix = cameraProjectionMatrix * (*camera->getViewMatrix()) * light->getWorldMatrix();
		}

		auto dst = ((char*)memory) + i * buffers->getDataAlignment();
		memcpy(dst, &lightMatrix, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory()).setSize(sizeof(LightingPassVertexDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory());


	// lighting pass fragment dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory(), 0, sizeof(LightingPassFragmentDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		// TODO: replace this code, put data into light
		glm::mat4 lightData;
		lightData[0] = glm::vec4(light->position, light->type == LightType::Directional ? 0.0f : 1.0f);
		lightData[1] = glm::vec4(light->color, light->intensity);
		lightData[2] = glm::vec4(light->getRange(), light->specularPower, light->castShadows ? 1.0f: 0.0f, 0.0f);
		lightData[3] = glm::vec4(0.0f);

		auto dst = ((char*)memory) + i * buffers->getDataAlignment();
		memcpy(dst, &lightData, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory()).setSize(sizeof(LightingPassFragmentDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory());


	// lighting pass fragment uniform buffer

	buffers->getLightingPassFragmentData()->data[0] = glm::vec4(camera.get()->position, 0.0f);
	buffers->getLightingPassFragmentData()->data[1] = glm::vec4(window->getWidth(), window->getHeight(), 0.0f, 0.0f);
	memory = context->getDevice()->mapMemory(*buffers->getLightingPassFragmentUniformBufferMemory(), 0, sizeof(LightingPassFragmentData));
	memcpy(memory, buffers->getLightingPassFragmentData(), sizeof(LightingPassFragmentData));
	context->getDevice()->unmapMemory(*buffers->getLightingPassFragmentUniformBufferMemory());

#endif
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