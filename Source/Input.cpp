#include "Input.hpp"

Input::Input(const std::shared_ptr<Window> window)
{
	this->window = window;

	windowHalfWidth = window->getWidth() / 2;
	windowHalfHeight = window->getHeight() / 2;

	forwardKeyPressed = backKeyPressed = leftKeyPressed = rightKeyPressed = upKeyPressed = downKeyPressed = crouchKeyPressed = flyKeyPressed = leftMouseButtonPressed = rightMouseButtonPressed = false;
	mouseDelta = glm::vec2(0.0f);
}

void Input::sendMouseMoveEvent(const SDL_Event &event)
{
	mouseDelta.x += (event.motion.x - windowHalfWidth) / (float)windowHalfWidth;
	mouseDelta.y += (event.motion.y - windowHalfHeight) / (float)windowHalfHeight;
}

void Input::sendMouseButtonEvent(const SDL_Event &event)
{
	if (event.button.button == SDL_BUTTON_LEFT)
	{
		leftMouseButtonPressed = event.type == SDL_MOUSEBUTTONDOWN;
	}
	else if (event.button.button == SDL_BUTTON_RIGHT)
	{
		rightMouseButtonPressed = event.type == SDL_MOUSEBUTTONDOWN;
	}
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
	else if (event.key.keysym.sym == SDLK_SPACE)
		upKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_LCTRL)
		downKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_LSHIFT)
		crouchKeyPressed = event.type == SDL_KEYDOWN;
	else if (event.key.keysym.sym == SDLK_f)
		flyKeyPressed = event.type == SDL_KEYDOWN;
}

void Input::resetMouseMovement()
{
	mouseDelta = glm::vec2(0.0f);
	//SDL_WarpMouseInWindow(window->getWindow(), windowHalfWidth, windowHalfHeight);
}