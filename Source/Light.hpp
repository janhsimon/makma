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
	float range, intensity, specularPower;

	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, float specularPower);
};
