#include "DirectionalLight.hpp"

DirectionalLight::DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color)
{
	this->direction = direction;
	this->color = color;
}