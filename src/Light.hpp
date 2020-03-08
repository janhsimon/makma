#pragma once

#include "Transform.hpp"
#include "Renderer/ShadowPass/ShadowMap.hpp"

enum LightType
{
	Directional,
	Point,
	Spot
};

class Light : public Transform
{
private:
	float range;

public:
	LightType type;
	glm::vec3 color;
	float intensity;
	float spotAngle;
	bool castShadows;
	std::shared_ptr<ShadowMap> shadowMap;
	
	void DirectionalLight(const glm::vec3 &position, const glm::vec3 &eulerAngles, const glm::vec3 &color, float intensity, bool castShadows);
	void PointLight(const glm::vec3 &position, const glm::vec3 &color, float range, float intensity);
	void SpotLight(const glm::vec3 &position, const glm::vec3 &eulerAngles, const glm::vec3 &color, float range, float intensity, float spotAngle);

	glm::mat4 getData() const;

	float getRange() const { return range; }
	void setRange(float range);
};
