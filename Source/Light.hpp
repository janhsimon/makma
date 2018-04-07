#pragma once

#include "Transform.hpp"
#include "Renderer/ShadowPass/ShadowMap.hpp"

enum LightType
{
	Directional,
	Point
};

class Light : public Transform
{
private:
	float range;

public:
	LightType type;
	glm::vec3 color;
	float intensity;
	bool castShadows;
	std::shared_ptr<ShadowMap> shadowMap;
	
	Light(LightType type, const glm::vec3 &position, const glm::vec3 &color, float range, float intensity, bool castShadows);

	glm::mat4 getData() const;

	float getRange() const { return range; }
	void setRange(float range);
};
