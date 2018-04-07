#include "Light.hpp"

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, bool castShadows)
{
	this->type = type;
	this->position = position;
	this->color = color;
	this->intensity = intensity;
	this->castShadows = castShadows;

	setRange(range);
}

glm::mat4 Light::getData() const
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