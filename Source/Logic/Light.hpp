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

	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color = glm::vec3(1.0f, 1.0f, 1.0f), float range = 1000.0f, float intensity = 1.0f, float specularPower = 4.0f);
};
