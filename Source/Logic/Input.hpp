#pragma once

#include "..\Window.hpp"

#include <glm.hpp>

class Input
{
private:
	std::shared_ptr<Window> window;

	unsigned short windowHalfWidth, windowHalfHeight;

public:
	bool forwardKeyPressed;
	bool backKeyPressed;
	bool leftKeyPressed;
	bool rightKeyPressed;
	bool upKeyPressed;
	bool downKeyPressed;
	bool crouchKeyPressed;

	glm::vec2 mouseDelta;

	Input(const std::shared_ptr<Window> window);

	void sendMouseMoveEvent(const SDL_Event &event);
	void sendKeyboardEvent(const SDL_Event &event);

	void resetMouseMovement();
};