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

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	uniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, sizeof(UniformBufferData), false, vk::ShaderStageFlagBits::eVertex, sizeof(UniformBufferData));
	dynamicUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, numShadowMaps * context->getUniformBufferDataAlignment() + static_cast<uint32_t>(modelList.size()) * context->getUniformBufferDataAlignment() + 2 * static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true, vk::ShaderStageFlagBits::eAllGraphics, sizeof(DynamicUniformBufferData));
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	shadowPassDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, numShadowMaps * context->getUniformBufferDataAlignment(), true, vk::ShaderStageFlagBits::eAllGraphics, sizeof(ShadowPassDynamicUniformBufferData));
	geometryPassVertexDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, static_cast<uint32_t>(modelList.size()) * context->getUniformBufferDataAlignment(), true, vk::ShaderStageFlagBits::eVertex, sizeof(GeometryPassVertexDynamicUniformBufferData));
	geometryPassVertexUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, sizeof(GeometryPassVertexUniformBufferData), false, vk::ShaderStageFlagBits::eVertex, sizeof(GeometryPassVertexUniformBufferData));
	lightingPassVertexDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true, vk::ShaderStageFlagBits::eVertex, sizeof(LightingPassVertexDynamicUniformBufferData));
	lightingPassVertexUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, sizeof(LightingPassVertexUniformBufferData), false, vk::ShaderStageFlagBits::eVertex, sizeof(LightingPassVertexUniformBufferData));
	lightingPassFragmentDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, descriptorPool, static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true, vk::ShaderStageFlagBits::eFragment, sizeof(LightingPassFragmentDynamicUniformBufferData));
#endif

	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptorPool);
	}

	std::vector<vk::DescriptorSetLayout> setLayouts;
#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	setLayouts.push_back(*geometryPassVertexDynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*shadowPassDynamicUniformBuffer->getDescriptor()->getLayout());
#endif
	shadowPipeline = std::make_shared<ShadowPipeline>(context, setLayouts);

	uint32_t shadowMapIndex = 0;
	for (uint32_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList[i];

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
		light->finalize(context, vertexBuffer, indexBuffer, dynamicUniformBuffer, descriptorPool, shadowPipeline, &modelList, light->castShadows ? (shadowMapIndex++) : 0, numShadowMaps);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
		light->finalize(context, vertexBuffer, indexBuffer, shadowPassDynamicUniformBuffer, geometryPassVertexDynamicUniformBuffer, descriptorPool, shadowPipeline, &modelList, light->castShadows ? (shadowMapIndex++) : 0, numShadowMaps);
#endif
	}

	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptorPool);

	setLayouts.clear();
#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*uniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*descriptorPool->getMaterialLayout());
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	setLayouts.push_back(*geometryPassVertexDynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*geometryPassVertexUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*descriptorPool->getMaterialLayout());
#endif
	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, setLayouts, geometryBuffer->getRenderPass());

	swapchain = std::make_unique<Swapchain>(window, context, unitQuadModel, unitSphereModel);

	setLayouts.clear();
#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*uniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*descriptorPool->getGeometryBufferLayout());
	setLayouts.push_back(*descriptorPool->getShadowMapLayout());
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*dynamicUniformBuffer->getDescriptor()->getLayout());
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	setLayouts.push_back(*lightingPassVertexDynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*lightingPassVertexUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*descriptorPool->getGeometryBufferLayout());
	setLayouts.push_back(*descriptorPool->getShadowMapLayout());
	setLayouts.push_back(*lightingPassFragmentDynamicUniformBuffer->getDescriptor()->getLayout());
	setLayouts.push_back(*shadowPassDynamicUniformBuffer->getDescriptor()->getLayout());
#endif
	lightingPipeline = std::make_shared<LightingPipeline>(window, context, setLayouts, swapchain->getRenderPass());

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptorPool, vertexBuffer, indexBuffer, uniformBuffer, dynamicUniformBuffer, &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()));
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	swapchain->recordCommandBuffers(lightingPipeline, geometryBuffer, descriptorPool, vertexBuffer, indexBuffer, shadowPassDynamicUniformBuffer, lightingPassVertexDynamicUniformBuffer, lightingPassVertexUniformBuffer, lightingPassFragmentDynamicUniformBuffer, &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()));
#endif

	// just record the command buffers once if we are not using push constants
#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer, dynamicUniformBuffer, &modelList, numShadowMaps);
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, geometryPassVertexDynamicUniformBuffer, geometryPassVertexUniformBuffer, &modelList, numShadowMaps);
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

	auto memory = context->getDevice()->mapMemory(*shadowPassDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(ShadowPassDynamicUniformBufferData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		if (light->castShadows)
		{
			auto dst = ((char*)memory) + i * context->getUniformBufferDataAlignment();
			memcpy(dst, &light->getShadowMap()->getViewProjectionMatrix(light->position), sizeof(glm::mat4));
		}
	}

	auto memoryRange = vk::MappedMemoryRange().setMemory(*shadowPassDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(ShadowPassDynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*shadowPassDynamicUniformBuffer->getBuffer()->getMemory());


	// geometry pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*geometryPassVertexDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(GeometryPassVertexDynamicUniformBufferData));
	for (size_t i = 0; i < modelList.size(); ++i)
	{
		auto dst = ((char*)memory) + i * context->getUniformBufferDataAlignment();
		memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*geometryPassVertexDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(GeometryPassVertexDynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*geometryPassVertexDynamicUniformBuffer->getBuffer()->getMemory());


	// geometry pass vertex uniform buffer

	auto cameraProjectionMatrix = *camera.get()->getProjectionMatrix();
	cameraProjectionMatrix[1][1] *= -1.0f;
	geometryPassVertexUniformBufferData.cameraViewProjectionMatrix = cameraProjectionMatrix * (*camera.get()->getViewMatrix());
	memory = context->getDevice()->mapMemory(*geometryPassVertexUniformBuffer->getBuffer()->getMemory(), 0, sizeof(GeometryPassVertexUniformBufferData));
	memcpy(memory, &geometryPassVertexUniformBufferData, sizeof(GeometryPassVertexUniformBufferData));
	context->getDevice()->unmapMemory(*geometryPassVertexUniformBuffer->getBuffer()->getMemory());


	// lighting pass vertex dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*lightingPassVertexDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(LightingPassVertexDynamicUniformBufferData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);

		auto lightWorldCameraViewProjectionMatrix = glm::mat4(1.0f);
		if (lightList[i]->type == LightType::Point)
		{
			lightWorldCameraViewProjectionMatrix = geometryPassVertexUniformBufferData.cameraViewProjectionMatrix * light->getWorldMatrix();
		}

		auto dst = ((char*)memory) + i * context->getUniformBufferDataAlignment();
		memcpy(dst, &lightWorldCameraViewProjectionMatrix, sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*lightingPassVertexDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(LightingPassVertexDynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*lightingPassVertexDynamicUniformBuffer->getBuffer()->getMemory());


	// lighting pass vertex uniform buffer

	lightingPassVertexUniformBufferData.globals[0] = glm::vec4(camera.get()->position, 0.0f);
	lightingPassVertexUniformBufferData.globals[1] = glm::vec4(window->getWidth(), window->getHeight(), 0.0f, 0.0f);
	memory = context->getDevice()->mapMemory(*lightingPassVertexUniformBuffer->getBuffer()->getMemory(), 0, sizeof(LightingPassVertexUniformBufferData));
	memcpy(memory, &lightingPassVertexUniformBufferData, sizeof(LightingPassVertexUniformBufferData));
	context->getDevice()->unmapMemory(*lightingPassVertexUniformBuffer->getBuffer()->getMemory());


	// lighting pass fragment dynamic uniform buffer

	memory = context->getDevice()->mapMemory(*lightingPassFragmentDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(LightingPassFragmentDynamicUniformBufferData));
	for (size_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList.at(i);
		auto dst = ((char*)memory) + i * context->getUniformBufferDataAlignment();
		memcpy(dst, &light->getEncodedData(), sizeof(glm::mat4));
	}

	memoryRange = vk::MappedMemoryRange().setMemory(*lightingPassFragmentDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(LightingPassFragmentDynamicUniformBufferData));
	context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
	context->getDevice()->unmapMemory(*lightingPassFragmentDynamicUniformBuffer->getBuffer()->getMemory());
	
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