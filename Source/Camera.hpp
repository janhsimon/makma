#pragma once

#include "Input.hpp"
#include "Transform.hpp"

#include <memory>
#include <vector>

enum CameraState
{
	Walking,
	Flying,
	OnRails
};

struct RailPoint
{
	glm::vec3 position;
	glm::vec3 eulerAngles;

	RailPoint(const glm::vec3 &position, const glm::vec3 &eulerAngles)
	{
		this->position = position;
		this->eulerAngles = eulerAngles;
	}
};

struct Rail
{
	std::vector<RailPoint> railPoints;
};

class Camera : public Transform
{
private:
	glm::mat4 viewMatrix, projectionMatrix;
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	CameraState state;
	float fov, nearClip, farClip, movementSpeed;
	bool firstFrame;
	glm::vec3 originalEulerAngles;
	std::vector<Rail> rails;
	float railTimer;
	uint32_t railIndex, railPointIndex;

public:
	float mouseSensitivity;

	Camera(const std::shared_ptr<Window> window, std::shared_ptr<Input> input, const glm::vec3 &position = glm::vec3(0.0f), const glm::vec3 &eulerAngles = glm::vec3(0.0f), float fov = 75.0f, float nearClip = 1.0f, float farClip = 1000.0f, float mouseSensitivity = 5.0f);
	
	void startRails() { window->setShowMouseCursor(false); state = CameraState::OnRails; }

	void update(float delta);
	
	void setFOV(float fov);
	float getNearClip() const { return nearClip; }
	float getFarClip() const { return farClip; }
	CameraState getState() const { return state; }

	glm::mat4 *getViewMatrix() { return &viewMatrix; };
	glm::mat4 *getProjectionMatrix() { return &projectionMatrix; };
};