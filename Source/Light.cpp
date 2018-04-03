#include "Light.hpp"
#include "Renderer/Settings.hpp"

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, bool castShadows)
{
	this->type = type;
	this->position = position;
	this->color = color;
	this->intensity = intensity;
	this->castShadows = castShadows;

	setRange(range);
}

#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
void Light::finalize(const std::shared_ptr<Context> context, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> dynamicUniformBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<ShadowPipeline> shadowPipeline, std::vector<std::shared_ptr<Model>> *models, uint32_t shadowMapIndex, uint32_t numShadowMaps)
#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
void Light::finalize(const std::shared_ptr<Context> context, const std::shared_ptr<VertexBuffer> vertexBuffer, const std::shared_ptr<IndexBuffer> indexBuffer, const std::shared_ptr<UniformBuffer> shadowPassDynamicUniformBuffer, const std::shared_ptr<UniformBuffer> geometryPassVertexDynamicUniformBuffer, const std::shared_ptr<DescriptorPool> descriptorPool, const std::shared_ptr<ShadowPipeline> shadowPipeline, const std::vector<std::shared_ptr<Model>> *models, uint32_t shadowMapIndex, uint32_t numShadowMaps)
#endif
{
	if (!castShadows)
	{
		return;
	}

	this->context = context;
	this->vertexBuffer = vertexBuffer;
	this->indexBuffer = indexBuffer;
	this->dynamicUniformBuffer = dynamicUniformBuffer;
	this->descriptorPool = descriptorPool;
	this->shadowPipeline = shadowPipeline;
	this->models = models;
	this->shadowMapIndex = shadowMapIndex;
	this->numShadowMaps = numShadowMaps;
}

void Light::buildShadowMap()
{
	if (!castShadows)
	{
		return;
	}

//#if MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC
	if (Settings::uniformBufferMode == SETTINGS_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC)
	{
		shadowMap = std::make_shared<ShadowMap>(context, vertexBuffer, indexBuffer, dynamicUniformBuffer, descriptorPool, shadowPipeline, models, shadowMapIndex, numShadowMaps);
	}
//#elif MK_OPTIMIZATION_UNIFORM_BUFFER_MODE == MK_OPTIMIZATION_UNIFORM_BUFFER_MODE_INDIVIDUAL
	else if (Settings::uniformBufferMode == SETTINGS_UNIFORM_BUFFER_MODE_INDIVIDUAL)
	{
		//shadowMap = std::make_shared<ShadowMap>(context, vertexBuffer, indexBuffer, shadowPassDynamicUniformBuffer, geometryPassVertexDynamicUniformBuffer, descriptorPool, shadowPipeline, models, shadowMapIndex, numShadowMaps);
	}
//#endif
}

glm::mat4 Light::getEncodedData() const
{
	glm::mat4 lightData;
	lightData[0] = glm::vec4(position, type == LightType::Directional ? 0.0f : 1.0f);
	lightData[1] = glm::vec4(color, intensity);
	lightData[2] = glm::vec4(getRange(), castShadows ? 1.0f : 0.0f, 0.0f, 0.0f);
	lightData[3] = glm::vec4(0.0f);
	return lightData;
}

void Light::setRange(float range)
{
	this->range = range;
	this->scale = glm::vec3(range);
}