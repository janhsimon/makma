#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // bring the depth range from [-1,1] (OpenGL) to [0,1] (Vulkan)
#include <gtc\matrix_transform.hpp>

Camera::Camera(const glm::vec3 &position, const std::shared_ptr<Window> window, const std::shared_ptr<Input> input) : Transform(position)
{
	this->input = input;

	viewMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::perspectiveFov(glm::radians(75.0f), static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()), 0.1f, 3000.0f);
	free = false;
}

void Camera::rotatePitch(float amount)
{
	setPitch(getPitch() + amount * 45.0f);

	const auto pitchLock = 90.0f;

	if (getPitch() < -pitchLock)
		setPitch(-pitchLock);
	else if (getPitch() > pitchLock)
		setPitch(pitchLock);
}

void Camera::rotateYaw(float amount)
{
	setYaw(getYaw() + amount * 45.0f);
}

void Camera::rotateRoll(float amount)
{
	setRoll(getRoll() + amount * 45.0f);
}

void Camera::update(float delta)
{
	auto forward = getForward();
	auto right = getRight();
	auto up = free ? getUp() : glm::vec3(0.0f, 1.0f, 0.0f);

	if (!free)
	{
		forward.y = 0.0f;
		right.y = 0.0f;
	}

	if (input->forwardKeyPressed && !input->backKeyPressed)
	{
		position += forward * 0.4f * delta * (input->crouchKeyPressed ? 0.4f : 1.0f);
	}
	else if (input->backKeyPressed && !input->forwardKeyPressed)
	{
		position -= forward * 0.4f * delta * (input->crouchKeyPressed ? 0.4f : 1.0f);
	}

	if (input->leftKeyPressed && !input->rightKeyPressed)
	{
		position += right * 0.4f * delta * (input->crouchKeyPressed ? 0.4f : 1.0f);
	}
	else if (input->rightKeyPressed && !input->leftKeyPressed)
	{
		position -= right * 0.4f * delta * (input->crouchKeyPressed ? 0.4f : 1.0f);
	}

	if (free)
	{
		if (input->upKeyPressed && !input->downKeyPressed)
		{
			position += getUp() * 0.4f * delta * (input->crouchKeyPressed ? 0.4f : 1.0f);
		}
		else if (input->downKeyPressed && !input->upKeyPressed)
		{
			position -= getUp() * 0.4f * delta * (input->crouchKeyPressed ? 0.4f : 1.0f);
		}
	}

	rotateYaw(-input->mouseDelta.x);
	rotatePitch(input->mouseDelta.y);

	viewMatrix = glm::lookAt(position, position + getForward(), getUp());
}