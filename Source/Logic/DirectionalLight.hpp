#pragma once

#include <glm.hpp>

class DirectionalLight
{
public:
	glm::vec3 direction;
	glm::vec3 color;

	DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color);
};
