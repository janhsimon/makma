#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

class Transform
{
protected:
	glm::vec3 forward, right, up;
	float pitch, yaw, roll;

public:
	glm::vec3 position;
	glm::vec3 scale;

	Transform(glm::vec3 position = glm::vec3(0.f));

	void recalculateAxesFromAngles();

	glm::mat4 getWorldMatrix() const;

	float getPitch() const { return pitch; }
	void setPitch(float pitch) { this->pitch = pitch; };

	float getYaw() const { return yaw; }
	void setYaw(float yaw) { this->yaw = yaw; };

	float getRoll() const { return roll; }
	void setRoll(float roll) { this->roll = roll; };

	glm::vec3 getForward() const { return forward; }
	glm::vec3 getRight() const { return right; }
	glm::vec3 getUp() const { return up; }
};