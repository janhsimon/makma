#include "Camera.hpp"

#include <gtc/matrix_transform.hpp>

Camera::Camera(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const glm::vec3 &position, const glm::vec3 &eulerAngles, float fov, float nearClip, float farClip, float mouseSensitivity) : Transform(position)
{
	this->window = window;
	this->input = input;

	state = CameraState::Walking;
	
	this->nearClip = nearClip;
	this->farClip = farClip;
	this->mouseSensitivity = mouseSensitivity;

	movementSpeed = 0.5f;

	originalEulerAngles = eulerAngles;

	Rail rail;

	// upper level pillar fly-by shot
	rail.railPoints.push_back(RailPoint(glm::vec3(676.0f, 652.0f, 23.0f), glm::vec3(5.0f, 320.0f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(-503.0f, 594.0f, 51.5f), glm::vec3(0.45f, 310.0f, 0.0f)));
	rails.push_back(rail);

	// upper level shadow orbit shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(-660.0f, 675.45f, 451.6f), glm::vec3(11.5f, 195.0f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(-1287.25f, 518.5f, 142.25f), glm::vec3(25.7f, 117.3f, 0.0f)));
	rails.push_back(rail);

	// upper level spear fly-by shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(-702.0f, 952.0f, 53.0f), glm::vec3(70.0f, 128.0f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(305.5f, 962.0f, 42.0f), glm::vec3(79.5f, 225.0f, 0.0f)));
	rails.push_back(rail);

	// lower level fountain/lion orbit shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(-771.8f, 155.5f, -346.0f), glm::vec3(4.0f, -128.8f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(-1257.9f, 174.5f, -611.0f), glm::vec3(10.5f, 22.65f, 0.0f)));
	rails.push_back(rail);

	// lion backing away shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(1216.0f, 198.0f, -36.0f), glm::vec3(9.35f, 91.5f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(1110.0f, 213.5f, -36.0f), glm::vec3(6.5f, 92.5f, 0.0f)));
	rails.push_back(rail);

	// lower level pillar to pillar shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(-1148.25f, 167.95f, 261.2f), glm::vec3(25.3f, 139.9f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(-1275.45f, 196.68f, -274.5f), glm::vec3(20.8f, 33.8f, 0.0f)));
	rails.push_back(rail);

	// flower pot fly-by shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(-742.0f, 185.0f, 90.5f), glm::vec3(19.3f, 48.3f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(-589.5f, 169.0f, 49.5f), glm::vec3(7.3f, -12.25f, 0.0f)));
	rails.push_back(rail);

	// man shadow to face shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(-123.2f, 5.0f, 22.6f), glm::vec3(41.1f, 106.3f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(-25.72f, 175.9f, -4.15f), glm::vec3(18.76f, 102.3f, 0.0f)));
	rails.push_back(rail);

	// spinning up shot
	rail.railPoints.clear();
	rail.railPoints.push_back(RailPoint(glm::vec3(13.5f, 275.0f, 1.44f), glm::vec3(89.0f, 500.0f, 0.0f)));
	rail.railPoints.push_back(RailPoint(glm::vec3(13.5f, 700.0f, 1.44f), glm::vec3(89.0f, 2.5f, 0.0f)));
	rails.push_back(rail);

	railTimer = 0.0f;
	railIndex = railPointIndex = 0;

	setFOV(fov);

	firstFrame = true;
}

void Camera::setFOV(float fov)
{
	projectionMatrix = glm::perspectiveFov(glm::radians(fov), static_cast<float>(window->getWidth()), static_cast<float>(window->getHeight()), nearClip, farClip);
	projectionMatrix[1][1] *= -1.0f;
}

void Camera::update(float delta)
{
	if (state == CameraState::OnRails)
	{
		RailPoint from = rails.at(railIndex).railPoints.at(railPointIndex);
		RailPoint to = rails.at(railIndex).railPoints.at(railPointIndex + 1);

		position = glm::mix(from.position, to.position, railTimer);

		pitch = glm::mix(from.eulerAngles.x, to.eulerAngles.x, railTimer);
		yaw = glm::mix(from.eulerAngles.y, to.eulerAngles.y, railTimer);
		roll = glm::mix(from.eulerAngles.z, to.eulerAngles.z, railTimer);
		recalculateAxesFromAngles();

		railTimer += delta * 0.0001f;

		if (railTimer > 1.0f)
		{
			railPointIndex++;

			if (railPointIndex >= rails.at(railIndex).railPoints.size() - 1)
			{
				railIndex++;

				if (railIndex >= rails.size())
				{
					state = input->flyKeyPressed ? CameraState::Flying : CameraState::Walking;
					railTimer = 0.0f;
					railIndex = railPointIndex = 0;
				}

				railPointIndex = 0;
			}

			railTimer = 0.0f;
		}
	}
	else
	{
		auto mouseCursorVisible = window->getShowMouseCursor();
		if (input->showCursorKeyPressed && !mouseCursorVisible)
		{
			window->setShowMouseCursor(true);
		}
		else if (!input->showCursorKeyPressed && mouseCursorVisible)
		{
			window->setShowMouseCursor(false);
		}

		state = input->flyKeyPressed ? CameraState::Flying : CameraState::Walking;

		auto forward = getForward();
		auto right = getRight();
		auto up = getUp();

		if (state == CameraState::Walking)
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

		if (state == CameraState::Flying)
		{
			movementVector += up * (float)input->upKeyPressed;
			movementVector -= up * (float)input->downKeyPressed;
		}

		if (glm::length(movementVector) > 0.1f)
		{
			position += glm::normalize(movementVector) * movementSpeed * delta * (input->crouchKeyPressed ? 0.5f : 1.0f);
		}

		if (!input->showCursorKeyPressed)
		{
			yaw -= input->mouseDelta.x * mouseSensitivity * 0.01f;
			pitch += input->mouseDelta.y * mouseSensitivity * 0.01f;

			if (firstFrame)
			// we need to throw away the first frame mouse data because 
			// it contains a huge movement due to an SDL bug (?)
			// and that screw up camera rotations on the x-axis
			{
				pitch = originalEulerAngles.x;
				yaw = originalEulerAngles.y;
				roll = originalEulerAngles.z;
				firstFrame = false;
			}

			const auto PITCH_LOCK = 89.0f;

			if (pitch < -PITCH_LOCK)
				pitch = -PITCH_LOCK;
			else if (pitch > PITCH_LOCK)
				pitch = PITCH_LOCK;

			recalculateAxesFromAngles();
		}
	}

	viewMatrix = glm::lookAt(position, position + getForward(), getUp());
}