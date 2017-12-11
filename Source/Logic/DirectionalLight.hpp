#pragma once

#include <glm.hpp>

class DirectionalLight
{
private:
	glm::vec3 direction;
	glm::vec3 color;

public:
	DirectionalLight(const glm::vec3 &direction, const glm::vec3 &color);
};
