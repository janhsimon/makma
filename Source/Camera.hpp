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
	float fov;
	bool free;

	void rotatePitch(float amount);
	void rotateYaw(float amount);
	void rotateRoll(float amount);

public:
	Camera(const std::shared_ptr<Window> window, std::shared_ptr<Input> input, const glm::vec3 &position = glm::vec3(0.0f), float fov = 75.0f);
	
	void setFOV(float fov);
	void update(float delta);

	glm::mat4 *getViewMatrix() { return &viewMatrix; };
	glm::mat4 *getProjectionMatrix() { return &projectionMatrix; };
};