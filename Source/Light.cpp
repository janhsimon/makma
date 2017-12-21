#include "Light.hpp"

Light::Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, float specularPower)
{
	this->type = type;
	this->position = position;
	this->color = color;
	this->range = range;
	this->intensity = intensity;
	this->specularPower = specularPower;
}