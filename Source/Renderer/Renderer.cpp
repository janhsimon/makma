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

void Renderer::waitForQueueIdle()
{
	context->getQueue().waitIdle();
}

void Renderer::finalizeShadowPass()
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
		
		light->shadowMap = std::make_shared<ShadowMap>(context, descriptorPool, shadowPipeline);

		if (Settings::reuseCommandBuffers)
		{
			if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
			{
				light->shadowMap->recordCommandBuffer(vertexBuffer, indexBuffer, dynamicUniformBuffer->getDescriptor(1)->getSet(), dynamicUniformBuffer->getDescriptor(2)->getSet(), shadowPipeline, &modelList, shadowMapIndex, numShadowMaps);
			}
			else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
			{
				light->shadowMap->recordCommandBuffer(vertexBuffer, indexBuffer, shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getSet(), geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), shadowPipeline, &modelList, shadowMapIndex, numShadowMaps);
			}
		}

		++shadowMapIndex;
	}
}

void Renderer::finalizeGeometryPass()
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

	if (Settings::reuseCommandBuffers)
	{
		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(2)->getSet(), &modelList, numShadowMaps);
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), &modelList, numShadowMaps);
		}
	}
}

void Renderer::finalizeLightingPass()
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

	if (Settings::reuseCommandBuffers)
	{
		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			lightingBuffer->recordCommandBuffers(lightingPipeline, geometryBuffer, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(1)->getSet(), dynamicUniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(3)->getSet(), dynamicUniformBuffer->getDescriptor(4)->getSet(), &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()), unitQuadModel, unitSphereModel);
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			lightingBuffer->recordCommandBuffers(lightingPipeline, geometryBuffer, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getSet(), shadowMapSplitDepthsDynamicUniformBuffer->getDescriptor(0)->getSet(), lightWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), lightDataDynamicUniformBuffer->getDescriptor(0)->getSet(), &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()), unitQuadModel, unitSphereModel);
		}
	}
}

void Renderer::finalizeCompositePass()
{
	swapchain = std::make_unique<Swapchain>(window, context);

	if (!swapchain->getSwapchain())
	// this means we minimized the window and there is nothing to render
	{
		return;
	}

	std::vector<vk::DescriptorSetLayout> setLayouts;
	setLayouts.push_back(*descriptorPool->getLightingBufferLayout());
	compositePipeline = std::make_shared<CompositePipeline>(window, context, setLayouts, swapchain->getRenderPass());
}

void Renderer::finalize()
{
	numShadowMaps = 0;
	for (auto &light : lightList)
	{
		if (light->castShadows)
		{
			++numShadowMaps;
		}
	}

	context->getDevice()->waitIdle();
	context->calculateUniformBufferDataAlignment();

	vertexBuffer->finalize(context);
	indexBuffer->finalize(context);

	descriptorPool = std::make_shared<DescriptorPool>(context, Material::getNumMaterials(), numShadowMaps);
	
	uniformBuffer = std::make_shared<UniformBuffer>(context, sizeof(UniformBufferData), false);
	uniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eVertex, sizeof(UniformBufferData));
	if (Settings::keepUniformBufferMemoryMapped) uniformBuffer->getBuffer()->mapMemory();

	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		vk::DeviceSize size = (numShadowMaps + static_cast<uint32_t>(modelList.size()) + 2 * static_cast<uint32_t>(lightList.size())) * context->getUniformBufferDataAlignment() + numShadowMaps * context->getUniformBufferDataAlignmentLarge();
		dynamicUniformBuffer = std::make_shared<UniformBuffer>(context, size, true);
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// shadow map split depths
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4) * Settings::shadowMapCascadeCount);	// shadow map cascade view projection matrices
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// geometry world matrix
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// light world matrix
		dynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));										// light data
		if (Settings::keepUniformBufferMemoryMapped) dynamicUniformBuffer->getBuffer()->mapMemory();
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		shadowMapSplitDepthsDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, numShadowMaps * context->getUniformBufferDataAlignment(), true);
		shadowMapSplitDepthsDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4)); // TODO: is eAllGraphics necessary here and just above?
		if (Settings::keepUniformBufferMemoryMapped) shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->mapMemory();

		shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, numShadowMaps * context->getUniformBufferDataAlignmentLarge(), true);
		shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
		if (Settings::keepUniformBufferMemoryMapped) shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->mapMemory();

		geometryWorldMatrixDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, static_cast<uint32_t>(modelList.size()) * context->getUniformBufferDataAlignment(), true);
		geometryWorldMatrixDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));
		if (Settings::keepUniformBufferMemoryMapped) geometryWorldMatrixDynamicUniformBuffer->getBuffer()->mapMemory();

		lightWorldMatrixDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true);
		lightWorldMatrixDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));
		if (Settings::keepUniformBufferMemoryMapped) lightWorldMatrixDynamicUniformBuffer->getBuffer()->mapMemory();

		lightDataDynamicUniformBuffer = std::make_shared<UniformBuffer>(context, static_cast<uint32_t>(lightList.size()) * context->getUniformBufferDataAlignment(), true);
		lightDataDynamicUniformBuffer->addDescriptor(descriptorPool, vk::ShaderStageFlagBits::eAllGraphics, sizeof(glm::mat4));
		if (Settings::keepUniformBufferMemoryMapped) lightDataDynamicUniformBuffer->getBuffer()->mapMemory();
	}

	for (auto &model : modelList)
	{
		model->finalizeMaterials(descriptorPool);
	}

	finalizeShadowPass();
	finalizeGeometryPass();
	finalizeLightingPass();
	finalizeCompositePass();
	
	if (!swapchain->getSwapchain())
	// this means we minimized the window and there is nothing to render
	{
		return;
	}


	// ui

	std::vector<vk::DescriptorSetLayout> setLayouts;
	setLayouts.push_back(*descriptorPool->getFontLayout());
	ui = std::make_shared<UI>(window, context, descriptorPool, setLayouts, swapchain->getRenderPass());
	ui->applyChanges = [this]()
	{
		if (Settings::windowWidth != window->getWidth() || Settings::windowHeight != window->getHeight())
		{
			window->setSize(Settings::windowWidth, Settings::windowHeight);
		}

		if (Settings::windowMode != window->getMode())
		{
			window->setMode(static_cast<WindowMode>(Settings::windowMode));
		}

		finalize();
	};

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

	if (!Settings::keepUniformBufferMemoryMapped) uniformBuffer->getBuffer()->mapMemory();
	memcpy(uniformBuffer->getBuffer()->getMemoryMappedLocation(), &uniformBufferData, sizeof(UniformBufferData));
	if (!Settings::keepUniformBufferMemoryMapped) uniformBuffer->getBuffer()->unmapMemory();


	// dynamic uniform buffer

	if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
	{
		if (!Settings::keepUniformBufferMemoryMapped) dynamicUniformBuffer->getBuffer()->mapMemory();
		auto dst = (char*)dynamicUniformBuffer->getBuffer()->getMemoryMappedLocation();

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
		if (!Settings::keepUniformBufferMemoryMapped) dynamicUniformBuffer->getBuffer()->unmapMemory();
	}
	else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
	{
		// shadow map split depths

		if (!Settings::keepUniformBufferMemoryMapped) shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->mapMemory();
		auto dst = (char*)shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->getMemoryMappedLocation();

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

		if (Settings::flushDynamicUniformBufferMemoryIndividually)
		{
			auto memoryRange = vk::MappedMemoryRange().setMemory(*shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
			context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		}

		if (!Settings::keepUniformBufferMemoryMapped) shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->unmapMemory();


		// shadow map cascade view projection matrices

		if (!Settings::keepUniformBufferMemoryMapped) shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->mapMemory();
		dst = (char*)shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->getMemoryMappedLocation();

		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			if (light->shadowMap)
			{
				memcpy(dst, light->shadowMap->getCascadeViewProjectionMatrices(), sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
				dst += context->getUniformBufferDataAlignmentLarge();
			}
		}

		if (Settings::flushDynamicUniformBufferMemoryIndividually)
		{
			auto memoryRange = vk::MappedMemoryRange().setMemory(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4) * Settings::shadowMapCascadeCount);
			context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		}

		if (!Settings::keepUniformBufferMemoryMapped) shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->unmapMemory();


		// geometry world matrix

		if (!Settings::keepUniformBufferMemoryMapped) geometryWorldMatrixDynamicUniformBuffer->getBuffer()->mapMemory();
		dst = (char*)geometryWorldMatrixDynamicUniformBuffer->getBuffer()->getMemoryMappedLocation();

		for (size_t i = 0; i < modelList.size(); ++i)
		{
			memcpy(dst, &modelList.at(i)->getWorldMatrix(), sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		if (Settings::flushDynamicUniformBufferMemoryIndividually)
		{
			auto memoryRange = vk::MappedMemoryRange().setMemory(*geometryWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
			context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		}

		if (!Settings::keepUniformBufferMemoryMapped) geometryWorldMatrixDynamicUniformBuffer->getBuffer()->unmapMemory();


		// light world matrix

		if (!Settings::keepUniformBufferMemoryMapped) lightWorldMatrixDynamicUniformBuffer->getBuffer()->mapMemory();
		dst = (char*)lightWorldMatrixDynamicUniformBuffer->getBuffer()->getMemoryMappedLocation();

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

		if (Settings::flushDynamicUniformBufferMemoryIndividually)
		{
			auto memoryRange = vk::MappedMemoryRange().setMemory(*lightWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
			context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		}

		if (!Settings::keepUniformBufferMemoryMapped) lightWorldMatrixDynamicUniformBuffer->getBuffer()->unmapMemory();


		// lighting data

		if (!Settings::keepUniformBufferMemoryMapped) lightDataDynamicUniformBuffer->getBuffer()->mapMemory();
		dst = (char*)lightDataDynamicUniformBuffer->getBuffer()->getMemoryMappedLocation();

		for (size_t i = 0; i < lightList.size(); ++i)
		{
			const auto light = lightList.at(i);
			memcpy(dst, &light->getData(), sizeof(glm::mat4));
			dst += context->getUniformBufferDataAlignment();
		}

		if (Settings::flushDynamicUniformBufferMemoryIndividually)
		{
			auto memoryRange = vk::MappedMemoryRange().setMemory(*lightDataDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4));
			context->getDevice()->flushMappedMemoryRanges(1, &memoryRange);
		}

		if (!Settings::keepUniformBufferMemoryMapped) lightDataDynamicUniformBuffer->getBuffer()->unmapMemory();

		if (!Settings::flushDynamicUniformBufferMemoryIndividually)
		{
			std::vector<vk::MappedMemoryRange> mappedMemoryRanges;
			mappedMemoryRanges.push_back(vk::MappedMemoryRange().setMemory(*shadowMapSplitDepthsDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4)));
			mappedMemoryRanges.push_back(vk::MappedMemoryRange().setMemory(*shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4) * Settings::shadowMapCascadeCount));
			mappedMemoryRanges.push_back(vk::MappedMemoryRange().setMemory(*geometryWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4)));
			mappedMemoryRanges.push_back(vk::MappedMemoryRange().setMemory(*lightWorldMatrixDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4)));
			mappedMemoryRanges.push_back(vk::MappedMemoryRange().setMemory(*lightDataDynamicUniformBuffer->getBuffer()->getMemory()).setSize(sizeof(glm::mat4)));
			context->getDevice()->flushMappedMemoryRanges(static_cast<uint32_t>(mappedMemoryRanges.size()), mappedMemoryRanges.data());
		}
	}
}

void Renderer::render()
{
	if (swapchain->getSwapchainExtent().width <= 0 || swapchain->getSwapchainExtent().height <= 0)
	// this happens when we minimize the window for example										
	{
		// there is nothing to render
		return;
	}

	if (!Settings::reuseCommandBuffers)
	{
		vertexBuffer->finalize(context);
		indexBuffer->finalize(context);
	}

	swapchain->recordCommandBuffers(compositePipeline, lightingBuffer, vertexBuffer, indexBuffer, unitQuadModel, ui);

	
	// shadow pass

	auto submitInfo = vk::SubmitInfo().setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getShadowPassDoneSemaphore());
	std::vector<vk::CommandBuffer> commandBuffers;

	uint32_t shadowMapIndex = 0;
	for (auto &light : lightList)
	{
		if (light->shadowMap)
		{
			if (!Settings::reuseCommandBuffers)
			{
				if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
				{
					light->shadowMap->recordCommandBuffer(vertexBuffer, indexBuffer, dynamicUniformBuffer->getDescriptor(1)->getSet(), dynamicUniformBuffer->getDescriptor(2)->getSet(), shadowPipeline, &modelList, shadowMapIndex, numShadowMaps);
				}
				else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
				{
					light->shadowMap->recordCommandBuffer(vertexBuffer, indexBuffer, shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getSet(), geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), shadowPipeline, &modelList, shadowMapIndex, numShadowMaps);
				}
			}

			commandBuffers.push_back(*light->shadowMap->getCommandBuffer());
			++shadowMapIndex;
		}
	}

	submitInfo.setCommandBufferCount(static_cast<uint32_t>(commandBuffers.size())).setPCommandBuffers(commandBuffers.data());
	context->getQueue().submit({ submitInfo }, nullptr);


	// geometry pass

	if (!Settings::reuseCommandBuffers)
	{
		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(2)->getSet(), &modelList, numShadowMaps);
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			geometryBuffer->recordCommandBuffer(geometryPipeline, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), geometryWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), &modelList, numShadowMaps);
		}
	}

	submitInfo = vk::SubmitInfo().setSignalSemaphoreCount(1).setPSignalSemaphores(semaphores->getGeometryPassDoneSemaphore()).setCommandBufferCount(1).setPCommandBuffers(geometryBuffer->getCommandBuffer());

	if (Settings::renderMode == SETTINGS_RENDER_MODE_SERIAL)
	{
		vk::PipelineStageFlags stageFlags[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		submitInfo.setWaitSemaphoreCount(1).setPWaitSemaphores(semaphores->getShadowPassDoneSemaphore()).setPWaitDstStageMask(stageFlags);
	}

	context->getQueue().submit({ submitInfo }, nullptr);


	// lighting pass

	if (!Settings::reuseCommandBuffers)
	{
		if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL)
		{
			lightingBuffer->recordCommandBuffers(lightingPipeline, geometryBuffer, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(1)->getSet(), dynamicUniformBuffer->getDescriptor(0)->getSet(), dynamicUniformBuffer->getDescriptor(3)->getSet(), dynamicUniformBuffer->getDescriptor(4)->getSet(), &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()), unitQuadModel, unitSphereModel);
		}
		else if (Settings::dynamicUniformBufferStrategy == SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL)
		{
			lightingBuffer->recordCommandBuffers(lightingPipeline, geometryBuffer, vertexBuffer, indexBuffer, uniformBuffer->getDescriptor(0)->getSet(), shadowMapCascadeViewProjectionMatricesDynamicUniformBuffer->getDescriptor(0)->getSet(), shadowMapSplitDepthsDynamicUniformBuffer->getDescriptor(0)->getSet(), lightWorldMatrixDynamicUniformBuffer->getDescriptor(0)->getSet(), lightDataDynamicUniformBuffer->getDescriptor(0)->getSet(), &lightList, numShadowMaps, static_cast<uint32_t>(modelList.size()), unitQuadModel, unitSphereModel);
		}
	}

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
	if (nextImage.result == vk::Result::eErrorOutOfDateKHR)
	{
		// recreate the swapchain
		finalize();
		return;
	}
	else if (nextImage.result != vk::Result::eSuccess && nextImage.result != vk::Result::eSuboptimalKHR)
	{
		throw std::runtime_error("Failed to acquire next swapchain image for rendering.");
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
	auto presentResult = context->getQueue().presentKHR(&presentInfo);
	if (presentResult == vk::Result::eErrorOutOfDateKHR || presentResult == vk::Result::eSuboptimalKHR)
	{
		// also recreate the swapchain
		finalize();
		return;
	}
	else if (presentResult != vk::Result::eSuccess)
	{
		throw std::runtime_error("Failed to present rendered frame.");
	}

	waitForQueueIdle();
}
