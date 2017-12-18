#pragma once

#include <glm.hpp>

enum LightType
{
	Directional,
	Point
};

class Light
{
public:
	LightType type;
	glm::vec3 position;
	glm::vec3 color;
	float diffuseIntensity, specularIntensity, specularPower;

	Light(LightType type, const glm::vec3 &position);
	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color);
	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float diffuseIntensity);
	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float diffuseIntensity, float specularIntensity);
};
