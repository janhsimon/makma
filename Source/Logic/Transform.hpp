#pragma once

#include <glm.hpp>

class Transform
{
private:
	glm::vec3 forward, right, up;
	float pitch, yaw, roll;

	void recalculateAxisFromPitchYawRoll();

public:
	glm::vec3 position, velocity, acceleration;
	

	Transform(glm::vec3 position = glm::vec3(0.f));

	void updateTransform(float delta);

	glm::mat4 getWorldMatrix() const;

	inline float getPitch() const { return pitch; }
	void setPitch(float pitch);

	inline float getYaw() const { return yaw; }
	void setYaw(float yaw);

	inline float getRoll() const { return roll; }
	void setRoll(float roll);

	inline glm::vec3 getForward() const { return forward; }
	inline glm::vec3 getRight() const { return right; }
	inline glm::vec3 getUp() const { return up; }
};