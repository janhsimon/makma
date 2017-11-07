#pragma once

#include "Input.hpp"
#include "Transform.hpp"

#include <memory>

class Camera : public Transform
{
private:
	glm::mat4 viewMatrix, projectionMatrix;
	std::shared_ptr<Input> input;

	void rotatePitch(float amount);
	void rotateYaw(float amount);
	void rotateRoll(float amount);

public:
	Camera(const glm::vec3 &position, std::shared_ptr<Input> input);
	
	void update(float delta);

	glm::mat4 *getViewMatrix() { return &viewMatrix; };
	glm::mat4 *getProjectionMatrix() { return &projectionMatrix; };
};