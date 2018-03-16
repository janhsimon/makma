#include "Camera.hpp"

#include <gtc/matrix_transform.hpp>

Camera::Camera(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const glm::vec3 &position, float fov, float nearClip, float farClip, float mouseSensitivity) : Transform(position)
{
	this->window = window;
	this->input = input;
	
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->mouseSensitivity = mouseSensitivity;

	movementSpeed = 0.5f;

	setFOV(fov);

	free = false;
}

void Camera::setFOV(float fov)
{
	projectionMatrix = glm::perspectiveFov(glm::radians(fov), static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()), nearClip, farClip);
	projectionMatrix[1][1] *= -1.0f;
}

void Camera::update(float delta)
{
	if (input->lockKeyPressed)
	{
		SDL_ShowCursor(true);
		return;
	}

	if (SDL_ShowCursor(-1))
	{
		SDL_ShowCursor(false);
		input->resetMouseMovement();
	}

	free = input->flyKeyPressed;
	
	auto forward = getForward();
	auto right = getRight();
	auto up = getUp();

	if (!free)
	{
		forward.y = 0.0f;
		forward = glm::normalize(forward);
		
		right.y = 0.0f;
		
		up = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	auto movementVector = glm::vec3(0.0f);
	movementVector += forward * (float)input->forwardKeyPressed;
	movementVector -= forward * (float)input->backKeyPressed;
	movementVector += right * (float)input->leftKeyPressed;
	movementVector -= right * (float)input->rightKeyPressed;
	
	if (free)
	{
		movementVector += up * (float)input->upKeyPressed;
		movementVector -= up * (float)input->downKeyPressed;
	}

	if (glm::length(movementVector) > 0.1f)
	{
		position += glm::normalize(movementVector) * movementSpeed * delta * (input->crouchKeyPressed ? 0.5f : 1.0f);
	}

	yaw -= input->mouseDelta.x * mouseSensitivity;
	pitch += input->mouseDelta.y * mouseSensitivity;
	
	const auto PITCH_LOCK = 89.0f;

	if (pitch < -PITCH_LOCK)
		pitch = -PITCH_LOCK;
	else if (pitch > PITCH_LOCK)
		pitch = PITCH_LOCK;

	recalculateAxesFromAngles();

	viewMatrix = glm::lookAt(position, position + getForward(), getUp());
}