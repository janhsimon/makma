#include "Light.hpp"

Light::Light(LightType type, const glm::vec3 &position)
{
	this->type = type;
	this->position = position;
	this->color = glm::vec3(1.0f, 1.0f, 1.0f);

	diffuseIntensity = specularIntensity = 1000.0f;
	specularPower = 3.0f;
}

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color) : Light(type, position)
{
	this->color = color;
}

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float diffuseIntensity) : Light(type, position, color)
{
	this->diffuseIntensity = diffuseIntensity;
}

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float diffuseIntensity, float specularIntensity) : Light(type, position, color, diffuseIntensity)
{
	this->specularIntensity = specularIntensity;
}