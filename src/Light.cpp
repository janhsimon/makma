#include "Light.hpp"

void Light::DirectionalLight(const glm::vec3& position,
                             const glm::vec3& eulerAngles,
                             const glm::vec3& color,
                             float intensity,
                             bool castShadows)
{
  type = LightType::Directional;

  this->position = position;
  this->color = color;
  this->intensity = intensity;
  this->castShadows = castShadows;

  setPitch(eulerAngles.x);
  setYaw(eulerAngles.y);
  setRoll(eulerAngles.z);
  recalculateAxesFromAngles();
}

void Light::PointLight(const glm::vec3& position, const glm::vec3& color, float range, float intensity)
{
  type = LightType::Point;

  this->position = position;
  this->color = color;
  this->intensity = intensity;

  setRange(range);
}

void Light::SpotLight(const glm::vec3& position,
                      const glm::vec3& eulerAngles,
                      const glm::vec3& color,
                      float range,
                      float intensity,
                      float spotAngle)
{
  type = LightType::Spot;

  this->position = position;
  this->color = color;
  this->intensity = intensity;
  this->spotAngle = spotAngle;

  setPitch(eulerAngles.x);
  setYaw(eulerAngles.y);
  setRoll(eulerAngles.z);
  recalculateAxesFromAngles();

  setRange(range);
}

glm::mat4 Light::getData() const
{
  glm::mat4 lightData;
  lightData[0] = glm::vec4(position, static_cast<float>(type));
  lightData[1] = glm::vec4(forward, getRange());
  lightData[2] = glm::vec4(color, intensity);
  lightData[3] = glm::vec4(castShadows ? 1.0f : 0.0f, glm::cos(glm::radians(spotAngle)), 0.0f, 0.0f);
  return lightData;
}

void Light::setRange(float range)
{
  this->range = range;
  this->scale = glm::vec3(range);
}