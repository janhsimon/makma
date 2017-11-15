#include "Transform.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // bring the depth range from [-1,1] (OpenGL) to [0,1] (Vulkan)
#include <gtc\matrix_transform.hpp>
#include <gtc\quaternion.hpp>

void Transform::recalculateAxisFromPitchYawRoll()
{
	glm::mat4 orientationMatrix = glm::mat4_cast(glm::fquat(glm::vec3(glm::radians(pitch), glm::radians(yaw), glm::radians(roll))));
	forward = glm::normalize(glm::vec3(orientationMatrix * glm::vec4(0.0f, 0.0f, 1.0f, 0.0f)));
	right = glm::normalize(glm::vec3(orientationMatrix * glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)));
	up = glm::normalize(glm::vec3(orientationMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f)));
}

Transform::Transform(glm::vec3 position)
{
	this->position = position;

	velocity = acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	pitch = yaw = roll = 0.0f;

	forward = glm::vec3(0.0f, 0.0f, 1.0f);
	right = glm::vec3(1.0f, 0.0f, 0.0f);
	up = glm::vec3(0.0f, 1.0f, 0.0f);
}

void Transform::updateTransform(float delta)
{
	velocity += acceleration * delta;
	velocity *= 0.1f * delta;
	position += velocity * delta;
	
	recalculateAxisFromPitchYawRoll();
}

glm::mat4 Transform::getWorldMatrix() const
{
	auto worldMatrix = glm::mat4(1.0f);

	worldMatrix = glm::translate(worldMatrix, position);

	worldMatrix = glm::rotate(worldMatrix, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
	worldMatrix = glm::rotate(worldMatrix, glm::radians(roll), glm::vec3(0.0f, 0.0f, 1.0f));

	return worldMatrix;
}

void Transform::setPitch(float pitch)
{
	this->pitch = pitch;
	recalculateAxisFromPitchYawRoll();
}

void Transform::setYaw(float yaw)
{
	this->yaw = yaw;
	recalculateAxisFromPitchYawRoll();
}

void Transform::setRoll(float roll)
{
	this->roll = roll;
	recalculateAxisFromPitchYawRoll();
}