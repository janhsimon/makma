#include "Camera.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // bring the depth range from [-1,1] (OpenGL) to [0,1] (Vulkan)
#include <gtc\matrix_transform.hpp>

Camera::Camera(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const glm::vec3 &position, float fov) : Transform(position)
{
	this->window = window;
	this->input = input;

	projectionMatrix = glm::perspectiveFov(glm::radians(fov), static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()), 0.1f, 3000.0f);

	free = false;
}

void Camera::rotatePitch(float amount)
{
	setPitch(getPitch() + amount * 45.0f);

	const auto pitchLock = 89.0f;

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

void Camera::setFOV(float fov)
{
	projectionMatrix = glm::perspectiveFov(glm::radians(fov), static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()), 0.1f, 3000.0f);
}

void Camera::update(float delta)
{
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

	glm::vec3 movementVector;
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
		position += glm::normalize(movementVector) * 0.5f * delta * (input->crouchKeyPressed ? 0.5f : 1.0f);
	}

	rotateYaw(-input->mouseDelta.x);
	rotatePitch(input->mouseDelta.y);

	viewMatrix = glm::lookAt(position, position + getForward(), getUp());
}