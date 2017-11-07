#include "Input.hpp"

Input::Input(const std::shared_ptr<Window> window)
{
	this->window = window;

	windowHalfWidth = window->getWidth() / 2;
	windowHalfHeight = window->getHeight() / 2;

	forwardKeyPressed = backKeyPressed = leftKeyPressed = rightKeyPressed = upKeyPressed = downKeyPressed = crouchKeyPressed = false;
	mouseDelta = glm::vec2(0.0f);
}

void Input::sendMouseMoveEvent(const SDL_Event &event)
{
	mouseDelta.x += (event.motion.x - windowHalfWidth) / (float)windowHalfWidth;
	mouseDelta.y += (event.motion.y - windowHalfHeight) / (float)windowHalfHeight;
}

void Input::sendKeyboardEvent(const SDL_Event &event)
{
	if (event.key.keysym.sym == SDLK_w)
		forwardKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_s)
		backKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_a)
		leftKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_d)
		rightKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_e)
		upKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_q)
		downKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_LSHIFT)
		crouchKeyPressed = event.type == SDL_KEYDOWN;
}

void Input::resetMouseMovement()
{
	mouseDelta = glm::vec2(0.0f);
	SDL_WarpMouseInWindow(window->getWindow(), windowHalfWidth, windowHalfHeight);
}