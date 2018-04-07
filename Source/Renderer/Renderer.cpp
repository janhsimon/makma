#include "Renderer.hpp"
#include "Settings.hpp"

#include <chrono>

#define GLM_FORCE_RADIANS
#include <gtc/matrix_transform.hpp>

Renderer::Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera)
{
	this->window = window;
	this->input = input;
	this->camera = camera;

	context = std::make_shared<Context>(window);
	vertexBuffer = std::make_shared<VertexBuffer>();
	indexBuffer = std::make_shared<IndexBuffer>();

	Material::loadDefaultTextures(context);

	unitQuadModel = std::make_shared<Model>(context, vertexBuffer, indexBuffer, "Models/UnitQuad/", "UnitQuad.obj");
	unitSphereModel = std::make_shared<Model>(context, vertexBuffer, indexBuffer, "Models/UnitSphere/", "UnitSphere.obj");
}

std::shared_ptr<Model> Renderer::loadModel(const std::string &path, const std::string &filename)
{
	auto model = std::make_shared<Model>(context, vertexBuffer, indexBuffer, path, filename);
	modelList.push_back(model);
	return model;
}

std::shared_ptr<Light> Renderer::loadLight(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, bool castShadows)
{
	auto light = std::make_shared<Light>(type, position, color, range, intensity, castShadows);
	lightList.push_back(light);
	return light;
}

void Renderer::finalizeShadowPass(const uint32_t numShadowMaps)
{
	std::vector<vk::DescriptorSetLayout> setLayouts;

	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(2)->getLayout()); // geometry world matrix
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(1)->getLayout()); // shadow map cascade view projection matrices
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		setLayouts.push_back(*geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getLayout());
		setLayouts.push_back(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getLayout());
	}

	shadowPipeline = std::make_shared<ShadowPipeline>(context, setLayouts);

	uint32_t shadowMapIndex = 0;
	for (uint32_t i = 0; i < lightList.size(); ++i)
	{
		const auto light = lightList[i];

		if (!light->castShadows)
		{
			continue;
		}
		
		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			light->shadowMap = std::make_shared<ShadowMap>(context, vertexBuffer, indexBuffer, dynamicUniformBuffer->getDescriptor(1)->getSet(), dynamicUniformBuffer->getDescriptor(2)->getSet(), descriptorPool, shadowPipeline, &modelList, shadowMapIndex, numShadowMaps);
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			light->shadowMap = std::make_shared<ShadowMap>(context, vertexBuffer, indexBuffer, shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getSet(), geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), descriptorPool, shadowPipeline, &modelList, shadowMapIndex, numShadowMaps);
		}

		++shadowMapIndex;
	}
}

void Renderer::finalizeGeometryPass(const uint32_t numShadowMaps)
{
	geometryBuffer = std::make_shared<GeometryBuffer>(window, context, descriptorPool);

	std::vector<vk::DescriptorSetLayout> setLayouts;
	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(2)->getLayout()); // geometry world matrix
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		setLayouts.push_back(*geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getLayout());
	}
	setLayouts.push_back(*uniformBuffer->getDescriptor(0)->getLayout());
	setLayouts.push_back(*descriptorPool->getMaterialLayout());

	geometryPipeline = std::make_shared<GeometryPipeline>(window, context, setLayouts, geometryBuffer->getRenderPass());

	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(2)->getSet(), &modelList, numShadowMaps);
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), &modelList, numShadowMaps);
	}
}

void Renderer::finalizeLightingPass(const uint32_t numShadowMaps)
{
	lightingBuffer = std::make_shared<LightingBuffer>(window, context, descriptorPool);

	std::vector<vk::DescriptorSetLayout> setLayouts;
	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(3)->getLayout()); // light world matrix
		setLayouts.push_back(*uniformBuffer->getDescriptor(0)->getLayout());
		setLayouts.push_back(*descriptorPool->getGeometryBufferLayout());
		setLayouts.push_back(*descriptorPool->getShadowMapLayout());
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(4)->getLayout()); // light data
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(1)->getLayout()); // shadow map cascade view projection matrices
		setLayouts.push_back(*dynamicUniformBuffer->getDescriptor(0)->getLayout()); // shadow map split depths
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		setLayouts.push_back(*lightWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getLayout());
		setLayouts.push_back(*uniformBuffer->getDescriptor(0)->getLayout());
		setLayouts.push_back(*descriptorPool->getGeometryBufferLayout());
		setLayouts.push_back(*descriptorPool->getShadowMapLayout());
		setLayouts.push_back(*lightDataDynamicUniformBuffer->getDescriptor(0)->getLayout());
		setLayouts.push_back(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getLayout());
		setLayouts.push_back(*shadowMapSplitDepthsDynamicUniformBuffer->getDescriptor(0)->getLayout());
	}

	lightingPipeline = std::make_shared<LightingPipeline>(window, context, setLayouts, lightingBuffer->getRenderPass());

	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		lightingBuffer->recordCommandBuffers(lightingPipeline, geometryBuffer, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(1)->getSet(), dynamicUniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(3)->getSet(), dynamicUniformBuffer->getDescriptor(4)->getSet(), &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()), unitQuadModel, unitSphereModel);
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		lightingBuffer->recordCommandBuffers(lightingPipeline, geometryBuffer, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getSet(), shadowMapSplitDepthsDynamicUniformBuffer->getDescriptor(0)->getSet(), lightWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), lightDataDynamicUniformBuffer->getDescriptor(0)->getSet(), &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()), unitQuadModel, unitSphereModel);
	}
}

void Renderer::finalizeCompositePass()
{
	swapchain = std::make_unique<Swapchain>(window, context);

	std::vector<vk::DescriptorSetLayout> setLayouts;
	setLayouts.push_back(*descriptorPool->getLightingBufferLayout());
	compositePipeline = std::make_shared<CompositePipeline>(window, context, setLayouts, swapchain->getRenderPass());
}

void Renderer::finalize()
{
	context->calculateUniformBufferDataAlignment();

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
	
	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		uniformBuffer = std::make_shared<UniformBuffer>(context, sizeof(UniformBufferData), false);
		uniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eVertex, sizeof(UniformBufferData));

		vk::DeviceSize size = (numShadowMaps + static_cast<uint32_t>(modelList.size()) + 2 * static_cast<uint32_t>(lightList.size())) * context->getUniformBufferDataAlignment() + numShadowMaps * context->getUniformBufferDataAlignmentLarge();
		dynamicUniformBuffer = std::make_shared<UniformBuffer>(context, size, true);
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// shadow map split depths
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4) * Settings::shadowMapCascadeCount);	// shadow map cascade view projection matrices
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// geometry world matrix
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// light world matrix
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// light data
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		shadowMapSplitDepthsDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, numShadowMaps * context->getUniformBufferDataAlignment(), true);
		shadowMapSplitDepthsDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4)); // TODO: is eAllGraphics necessary here and just above?
		
		shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, numShadowMaps * context->getUniformBufferDataAlignmentLarge(), true);
		shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
		
		geometryWorldMatrixDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, static_cast<uint32_t>(modelList.size()) * context->getUniformBufferDataAlignment(), true);
		geometryWorldMatrixDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));

		lightWorldMatrixDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true);
		lightWorldMatrixDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));

		lightDataDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true);
		lightDataDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));
	}

	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptorPool);
	}

	finalizeShadowPass(numShadowMaps);
	finalizeGeometryPass(numShadowMaps);
	finalizeLightingPass(numShadowMaps);
	finalizeCompositePass();
	
	// ui
	std::vector<vk::DescriptorSetLayout> setLayouts;
	setLayouts.push_back(*descriptorPool->getFontLayout());
	ui = std::make_shared<UI>(window, context, descriptorPool, setLayouts, swapchain->getRenderPass());
	ui->applyChanges = [this]() { finalize(); };

	//swapchain->recordCommandBuffers(compositePipeline, lightingBuffer, vertexBuffer, indexBuffer, unitQuadModel, ui);

	semaphores = std::make_unique<Semaphores>(context);
}

void Renderer::updateUI(float delta)
{
	ui->update(input, camera, lightList, shadowPipeline, compositePipeline, lightingBuffer, delta);
}

void Renderer::updateBuffers()
{
	// uniform buffer

	uniformBufferData.cameraViewProjectionMatrix = (*camera->getProjectionMatrix()) * (*camera->getViewMatrix());
	uniformBufferData.cameraPosition = glm::vec4(camera->position, 0.0f);
	uniformBufferData.cameraForward = glm::vec4(camera->getForward(), 0.0f);

	auto memory = context->getDevice()->mapMemory(*uniformBuffer->getBuffer()->getMemory(), 0, sizeof(UniformBufferData));
	memcpy(memory, &uniformBufferData, sizeof(UniformBufferData));
	context->getDevice()->unmapMemory(*uniformBuffer->getBuffer()->getMemory());


	// dynamic uniform buffer

	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		memory = context->getDevice()->mapMemory(*dynamicUniformBuffer->getBuffer()->getMemory(), 0, VK_WHOLE_SIZE);
		auto dst = ((char*)memory);

		// shadow map split depths
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			if (light->shadowMap)
			{
				light->shadowMap->update(camera, glm::normalize(light->position));
				memcpy(dst, light->shadowMap->getSplitDepths(), sizeof(glm::mat4));
				dst += context->getUniformBufferDataAlignment();
			}
		}

		// shadow map cascade view projection matrices
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			if (light->shadowMap)
			{
				memcpy(dst, light->shadowMap->getCascadeViewProjectionMatrices(), sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
				dst += context->getUniformBufferDataAlignmentLarge();
			}
		}

		// geometry world matrix
		for (size_t i = 0; i < modelList.size(); ++i)
		{
			auto worldMatrix = modelList.at(i)->getWorldMatrix();
			memcpy(dst, &worldMatrix, sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		// light world matrix
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);

			auto lightWorldMatrix = glm::mat4(1.0f);
			if (light->type == LightType::Point)
			{
				lightWorldMatrix = light->getWorldMatrix();
			}

			memcpy(dst, &lightWorldMatrix, sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		// light data
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			auto data = light->getData();
			memcpy(dst, &data, sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		auto memoryRange = vk::MappedMemoryRange().setMemory(*dynamicUniformBuffer->getBuffer()->getMemory()).setSize(VK_WHOLE_SIZE);
		context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		context->getDevice()->unmapMemory(*dynamicUniformBuffer->getBuffer()->getMemory());
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		// shadow map split depths

		auto memory = context->getDevice()->mapMemory(*shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(glm::mat4));
		auto dst = ((char*)memory);
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			if (light->shadowMap)
			{
				light->shadowMap->update(camera, glm::normalize(light->position));
				memcpy(dst, light->shadowMap->getSplitDepths(), sizeof(glm::mat4));
				dst += context->getUniformBufferDataAlignment();
			}
		}

		auto memoryRange = vk::MappedMemoryRange().setMemory(*shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
		context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		context->getDevice()->unmapMemory(*shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->getMemory());


		// shadow map cascade view projection matrices

		memory = context->getDevice()->mapMemory(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
		dst = ((char*)memory);
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			if (light->shadowMap)
			{
				memcpy(dst, light->shadowMap->getCascadeViewProjectionMatrices(), sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
				dst += context->getUniformBufferDataAlignmentLarge();
			}
		}

		memoryRange = vk::MappedMemoryRange().setMemory(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
		context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		context->getDevice()->unmapMemory(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->getMemory());


		// geometry world matrix

		memory = context->getDevice()->mapMemory(*geometryWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(glm::mat4));
		dst = ((char*)memory);
		for (size_t i = 0; i < modelList.size(); ++i)
		{
			memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		memoryRange = vk::MappedMemoryRange().setMemory(*geometryWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
		context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		context->getDevice()->unmapMemory(*geometryWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory());


		// light world matrix

		memory = context->getDevice()->mapMemory(*lightWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(glm::mat4));
		dst = ((char*)memory);
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);

			auto lightWorldCameraViewProjectionMatrix = glm::mat4(1.0f);
			if (light->type == LightType::Point)
			{
				lightWorldCameraViewProjectionMatrix = light->getWorldMatrix();
			}

			memcpy(dst, &lightWorldCameraViewProjectionMatrix, sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		memoryRange = vk::MappedMemoryRange().setMemory(*lightWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
		context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		context->getDevice()->unmapMemory(*lightWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory());


		// lighting data

		memory = context->getDevice()->mapMemory(*lightDataDynamicUniformBuffer->getBuffer()->getMemory(), 0, sizeof(glm::mat4));
		dst = ((char*)memory);
		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			memcpy(dst, &light->getData(), sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		memoryRange = vk::MappedMemoryRange().setMemory(*lightDataDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
		context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		context->getDevice()->unmapMemory(*lightDataDynamicUniformBuffer->getBuffer()->getMemory());
	}
}

void Renderer::render()
{
	swapchain->recordCommandBuffers(compositePipeline, lightingBuffer, vertexBuffer, indexBuffer, unitQuadModel, ui);

	// shadow pass

	auto submitInfo = vk::SubmitInfo().setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getShadowPassDoneSemaphore());
	std::vector<vk::CommandBuffer> commandBuffers;

	for (auto &light : lightList)
	{
		if (light->shadowMap)
		{
			commandBuffers.push_back(*light->shadowMap->getCommandBuffer());
		}
	}

	submitInfo.setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size())).setPCommandBuffers(commandBuffers.data());
	context->getQueue().submit({ submitInfo }, nullptr);


	// geometry pass

	submitInfo = vk::SubmitInfo().setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getGeometryPassDoneSemaphore()).setCommandBufferCount(1).setPCommandBuffers(geometryBuffer->getCommandBuffer());

	if (Settings::renderMode == SETTINGS_RENDER_MODE_SERIAL)
	{
		vk::PipelineStageFlags stageFlags[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getShadowPassDoneSemaphore()).setPWaitDstStageMask(stageFlags);
	}

	context->getQueue().submit({ submitInfo }, nullptr);


	// lighting pass

	if (Settings::renderMode == SETTINGS_RENDER_MODE_PARALLEL)
	{
		vk::PipelineStageFlags stageFlags[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput };
		std::vector<vk::Semaphore> waitSemaphores = { *semaphores->getShadowPassDoneSemaphore(), *semaphores->getGeometryPassDoneSemaphore() };
		submitInfo = vk::SubmitInfo().setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size())).setPWaitSemaphores(waitSemaphores.data()).setPWaitDstStageMask(stageFlags);
		submitInfo.setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getLightingPassDoneSemaphore()).setCommandBufferCount(1).setPCommandBuffers(lightingBuffer->getCommandBuffer());
		context->getQueue().submit({ submitInfo }, nullptr);
	}
	else if (Settings::renderMode == SETTINGS_RENDER_MODE_SERIAL)
	{
		vk::PipelineStageFlags stageFlags[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo = vk::SubmitInfo().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getGeometryPassDoneSemaphore()).setPWaitDstStageMask(stageFlags);
		submitInfo.setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getLightingPassDoneSemaphore()).setCommandBufferCount(1).setPCommandBuffers(lightingBuffer->getCommandBuffer());
		context->getQueue().submit({ submitInfo }, nullptr);
	}


	// composite pass

	auto nextImage = context->getDevice()->acquireNextImageKHR(*swapchain->getSwapchain(), std::numeric_limits<uint64_t>::max(), *semaphores->getImageAvailableSemaphore(), nullptr);
	if (nextImage.result != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to acquire next image for rendering.");
	}

	auto imageIndex = nextImage.value;
	vk::PipelineStageFlags stageFlags2[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput };
	std::vector<vk::Semaphore> waitingForSemaphores = { *semaphores->getLightingPassDoneSemaphore(), *semaphores->getImageAvailableSemaphore() };
	submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitingForSemaphores.size())).setPWaitSemaphores(waitingForSemaphores.data()).setPWaitDstStageMask(stageFlags2);
	submitInfo.setPCommandBuffers(swapchain->getCommandBuffer(imageIndex)).setPSignalSemaphores(semaphores->getCompositePassDoneSemaphore());
	context->getQueue().submit({ submitInfo }, nullptr);


	// present

	auto presentInfo = vk::PresentInfoKHR().setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getCompositePassDoneSemaphore());
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
