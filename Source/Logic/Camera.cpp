#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // bring the depth range from [-1,1] (OpenGL) to [0,1] (Vulkan)
#include <gtc\matrix_transform.hpp>

Camera::Camera(const glm::vec3 &position, std::shared_ptr<Input> input) : Transform(position)
{
	this->input = input;

	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspectiveFov(glm::radians(90.0f), 1280.0f, 720.0f, 1.0f, 5000.0f);
}

void Camera::rotatePitch(float amount)
{
	setPitch(getPitch() + amount * 30.0f);

	const auto pitchLock = 90.0f;

	if (getPitch() < -pitchLock)
		setPitch(-pitchLock);
	else if (getPitch() > pitchLock)
		setPitch(pitchLock);
}

void Camera::rotateYaw(float amount)
{
	setYaw(getYaw() + amount * 30.0f);
}

void Camera::rotateRoll(float amount)
{
	setRoll(getRoll() + amount * 30.0f);
}

void Camera::update(float delta)
{
	if (input->forwardKeyPressed && !input->backKeyPressed)
		position += getForward() * 5.0f * delta * (input->crouchKeyPressed ? 0.25f : 1.0f);
	else if (input->backKeyPressed && !input->forwardKeyPressed)
		position -= getForward() * 5.0f * delta * (input->crouchKeyPressed ? 0.25f : 1.0f);

	if (input->leftKeyPressed && !input->rightKeyPressed)
		position += getRight() * 5.0f * delta * (input->crouchKeyPressed ? 0.25f : 1.0f);
	else if (input->rightKeyPressed && !input->leftKeyPressed)
		position -= getRight() * 5.0f * delta * (input->crouchKeyPressed ? 0.25f : 1.0f);

	if (input->upKeyPressed && !input->downKeyPressed)
		position += getUp() * 5.0f * delta * (input->crouchKeyPressed ? 0.25f : 1.0f);
	else if (input->downKeyPressed && !input->upKeyPressed)
		position -= getUp() * 5.0f * delta * (input->crouchKeyPressed ? 0.25f : 1.0f);

	rotateYaw(-input->mouseDelta.x);
	rotatePitch(input->mouseDelta.y);

	updateTransform(delta);

	viewMatrix = glm::lookAt(position, position + getForward(), getUp());
}