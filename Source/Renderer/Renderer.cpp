#include "Renderer.hpp"

#include <chrono>

#define GLM_FORCE_RADIANS
#include <gtc\matrix_transform.hpp>

Renderer::Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera)
{
	this->window = window;
	this->input = input;
	this->camera = camera;

	context = std::make_shared<Context>(window);
	vertexBuffer = std::make_shared<VertexBuffer>();
	indexBuffer = std::make_shared<IndexBuffer>();

	Material::loadDefaultTextures(context);

	unitQuadModel = std::make_shared<Model>(context, vertexBuffer, indexBuffer, "Models\\UnitQuad\\", "UnitQuad.obj");
	unitSphereModel = std::make_shared<Model>(context, vertexBuffer, indexBuffer, "Models\\UnitSphere\\", "UnitSphere.obj");
}

std::shared_ptr<Model> Renderer::loadModel(const std::string &path, const std::string &filename)
{
	auto model = std::make_shared<Model>(context, vertexBuffer, indexBuffer, path, filename);
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

	vertexBuffer->finalize(context);
	indexBuffer->finalize(context);

	descriptorPool = std::make_shared<DescriptorPool>(context, Material::getNumMaterials(), numShadowMaps);

	uniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, sizeof(UniformBufferData), false, vk::ShaderStageFlagBits::eVertex, sizeof(UniformBufferData));
	dynamicUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, numShadowMaps * context->getUniformBufferDataAlignment() + static_cast<uint32_t>(modelList.size()) * context->getUniformBufferDataAlignment() + 2 * static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true, vk::ShaderStageFlagBits::eAllGraphics, sizeof(DynamicUniformBufferData));

	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptorPool);
	}

	std::vector<vk::DescriptorSetLayout> setLayouts;
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	shadowPipeline = std::make_shared<ShadowPipeline>(context, setLayouts);

	uint32_t shadowMapIndex = 0;
	for (uint32_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList[i];
		light->finalize(context, vertexBuffer, indexBuffer, dynamicUniformBuffer, descriptorPool, shadowPipeline, &modelList, light->castShadows ? (shadowMapIndex++) : 0, numShadowMaps);
	}

	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptorPool);

	setLayouts.clear();
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*uniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*descriptorPool->getMaterialLayout());
	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, setLayouts, geometryBuffer->getRenderPass());

	swapchain = std::make_unique<Swapchain>(window, context, unitQuadModel, unitSphereModel);

	setLayouts.clear();
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*uniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*descriptorPool->getGeometryBufferLayout());
	setLayouts.push_back(*descriptorPool->getShadowMapLayout());
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	lightingPipeline = std::make_shared<LightingPipeline>(window, context, setLayouts, swapchain->getRenderPass());
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptorPool, vertexBuffer, indexBuffer, uniformBuffer, dynamicUniformBuffer, &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()));

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE != MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS
	// just record the command buffers once if we are not using push constants
	geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer, dynamicUniformBuffer, &modelList, numShadowMaps);
#endif

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::update()
{
#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC

	// uniform buffer

	auto cameraProjectionMatrix = *camera->getProjectionMatrix();
	cameraProjectionMatrix[1][1] *= -1.0f;
	uniformBufferData.cameraViewProjectionMatrix = cameraProjectionMatrix * (*camera->getViewMatrix());

	uniformBufferData.globals[0] = glm::vec4(camera->position, 0.0f);
	uniformBufferData.globals[1] = glm::vec4(window->getWidth(), window->getHeight(), 0.0f, 0.0f);

	auto memory = context->getDevice()->mapMemory(*uniformBuffer->getBuffer()->getMemory(), 0, sizeof(UniformBufferData));
	memcpy(memory, &uniformBufferData, sizeof(UniformBufferData));
	context->getDevice()->unmapMemory(*uniformBuffer->getBuffer()->getMemory());


	// dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*dynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(DynamicUniformBufferData));
	auto dst = ((char*)memory);
	
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		if (light->castShadows)
		{
			memcpy(dst, &light->getShadowMap()->getViewProjectionMatrix(light->position), sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}
	}

	for (size_t i = 0; i < modelList.size(); ++i)
	{
		memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
		dst += context->getUniformBufferDataAlignment();
	}

	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		auto lightWorldCameraViewProjectionMatrix = glm::mat4(1.0f);
		if (light->type == LightType::Point)
		{
			lightWorldCameraViewProjectionMatrix = uniformBufferData.cameraViewProjectionMatrix * light->getWorldMatrix();
		}

		memcpy(dst, &lightWorldCameraViewProjectionMatrix, sizeof(glm::mat4));
		dst += context->getUniformBufferDataAlignment();
	}

	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);
		memcpy(dst, &light->getEncodedData(), sizeof(glm::mat4));
		dst += context->getUniformBufferDataAlignment();
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*dynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(DynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*dynamicUniformBuffer->getBuffer()->getMemory());

#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL

	// shadow pass vertex dynamic uniform buffer

	auto memory = context->getDevice()->mapMemory(*buffers->getShadowPassVertexDynamicUniformBufferMemory(), 0, sizeof(ShadowPassDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		if (light->castShadows)
		{
			auto dst = ((char*)memory) + i * buffers->getDataAlignment();
			memcpy(dst, &light->getShadowMapViewProjectionMatrix(), sizeof(glm::mat4));
		}
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
	buffers->getGeometryPassVertexData()->cameraViewProjectionMatrix = cameraProjectionMatrix * (*camera.get()->getViewMatrix());
	memory = context->getDevice()->mapMemory(*buffers->getGeometryPassVertexUniformBufferMemory(), 0, sizeof(GeometryPassVertexData));
	memcpy(memory, buffers->getGeometryPassVertexData(), sizeof(GeometryPassVertexData));
	context->getDevice()->unmapMemory(*buffers->getGeometryPassVertexUniformBufferMemory());


	// lighting pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory(), 0, sizeof(LightingPassVertexDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		auto lightWorldCameraViewProjectionMatrix = glm::mat4(1.0f);
		if (lightList[i]->type == LightType::Point)
		{
			lightWorldCameraViewProjectionMatrix = buffers->getGeometryPassVertexData()->cameraViewProjectionMatrix * light->getWorldMatrix();
		}

		auto dst = ((char*)memory) + i * buffers->getDataAlignment();
		memcpy(dst, &lightWorldCameraViewProjectionMatrix, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory()).setSize(sizeof(LightingPassVertexDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightingPassVertexDynamicUniformBufferMemory());


	// lighting pass vertex uniform buffer

	buffers->getLightingPassVertexData()->globals[0] = glm::vec4(camera.get()->position, 0.0f);
	buffers->getLightingPassVertexData()->globals[1] = glm::vec4(window->getWidth(), window->getHeight(), 0.0f, 0.0f);
	memory = context->getDevice()->mapMemory(*buffers->getLightingPassVertexUniformBufferMemory(), 0, sizeof(LightingPassVertexData));
	memcpy(memory, buffers->getLightingPassVertexData(), sizeof(LightingPassVertexData));
	context->getDevice()->unmapMemory(*buffers->getLightingPassVertexUniformBufferMemory());


	// lighting pass fragment dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory(), 0, sizeof(LightingPassFragmentDynamicData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);
		auto dst = ((char*)memory) + i * buffers->getDataAlignment();
		memcpy(dst, &light->getEncodedData(), sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory()).setSize(sizeof(LightingPassFragmentDynamicData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*buffers->getLightingPassFragmentDynamicUniformBufferMemory());
	
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
			commandBuffers.push_back(*light->getShadowMap()->getCommandBuffer());
		}
	}

	submitInfo.setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size())).setPCommandBuffers(commandBuffers.data());
	context->getQueue().submit({ submitInfo }, nullptr);


	// geometry pass

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_PUSH_CONSTANTS
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