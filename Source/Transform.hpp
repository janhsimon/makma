#pragma once

#include <glm.hpp>

class Transform
{
private:
	glm::vec3 forward, right, up;
	float pitch, yaw, roll;

	void recalculateAxesFromPitchYawRoll();

public:
	glm::vec3 position;
	glm::vec3 scale;

	Transform(glm::vec3 position = glm::vec3(0.f));

	glm::mat4 getWorldMatrix() const;

	float getPitch() const { return pitch; }
	void setPitch(float pitch);

	float getYaw() const { return yaw; }
	void setYaw(float yaw);

	float getRoll() const { return roll; }
	void setRoll(float roll);

	glm::vec3 getForward() const { return forward; }
	glm::vec3 getRight() const { return right; }
	glm::vec3 getUp() const { return up; }
};