#include "Input.hpp"

Input::Input(const std::shared_ptr<Window> window)
{
	this->window = window;

	windowHalfWidth = window->getWidth() / 2;
	windowHalfHeight = window->getHeight() / 2;

	forwardKeyPressed = backKeyPressed = leftKeyPressed = rightKeyPressed = upKeyPressed = downKeyPressed = crouchKeyPressed = flyKeyPressed = showCursorKeyPressed = leftMouseButtonPressed = rightMouseButtonPressed = false;
	showControlsWindowKeyPressed = showParametersWindowKeyPressed = showLightEditorKeyPressed = showGraphsKeyPressed = true;
}

void Input::sendMouseMoveEvent(const SDL_Event &event)
{
	mouseDelta.x += event.motion.xrel;
	mouseDelta.y += event.motion.yrel;
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
	// held buttons

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
	

	// toggled buttons

	else if (event.key.keysym.sym == SDLK_f && event.type == SDL_KEYUP)
		flyKeyPressed = !flyKeyPressed;
	else if (event.key.keysym.sym == SDLK_TAB && event.type == SDL_KEYUP)
		showCursorKeyPressed = !showCursorKeyPressed;
	else if (event.key.keysym.sym == SDLK_c && event.type == SDL_KEYUP)
		showControlsWindowKeyPressed = !showControlsWindowKeyPressed;
	else if (event.key.keysym.sym == SDLK_p && event.type == SDL_KEYUP)
		showParametersWindowKeyPressed = !showParametersWindowKeyPressed;
	else if (event.key.keysym.sym == SDLK_l && event.type == SDL_KEYUP)
		showLightEditorKeyPressed = !showLightEditorKeyPressed;
	else if (event.key.keysym.sym == SDLK_g && event.type == SDL_KEYUP)
		showGraphsKeyPressed = !showGraphsKeyPressed;
}

void Input::resetMouseMovement()
{
	mouseDelta.x = mouseDelta.y = 0;
}