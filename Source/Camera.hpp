#pragma once

#include "Input.hpp"
#include "Transform.hpp"

#include <memory>

class Camera : public Transform
{
private:
	glm::mat4 viewMatrix, projectionMatrix;
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	float fov, nearClip, farClip, movementSpeed;
	bool free, firstFrame;

public:
	float mouseSensitivity;

	Camera(const std::shared_ptr<Window> window, std::shared_ptr<Input> input, const glm::vec3 &position = glm::vec3(0.0f), float fov = 75.0f, float nearClip = 1.0f, float farClip = 1000.0f, float mouseSensitivity = 5.0f);
	
	void update(float delta);
	
	void setFOV(float fov);
	float getNearClip() const { return nearClip; }
	float getFarClip() const { return farClip; }

	glm::mat4 *getViewMatrix() { return &viewMatrix; };
	glm::mat4 *getProjectionMatrix() { return &projectionMatrix; };
};