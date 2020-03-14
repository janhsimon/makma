#include "Window.hpp"

#include <stdexcept>
#include <vector>

SDL_Window *Window::createWindow(unsigned short width, unsigned short height, WindowMode mode)
{
	SDL_Window *window;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Failed to initialize SDL video subsystem, encountered error: " + std::string(SDL_GetError()));
	}

	auto flags = 0;
	if (mode == WindowMode::Fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}
	else if (mode == WindowMode::Borderless)
	{
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	window = SDL_CreateWindow("Makma", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN | flags);

	if (!window)
	{
		throw std::runtime_error("Failed to create window, encountered error: " + std::string(SDL_GetError()));
	}

	return window;
}

Window::Window(unsigned short width, unsigned short height, WindowMode mode)
{
	window = std::unique_ptr<SDL_Window, decltype(windowDeleter)>(createWindow(width, height, mode), windowDeleter);

	setShowMouseCursor(false);

	this->width = width;
	this->height = height;
	this->mode = mode;
}

void Window::setSize(unsigned short width, unsigned short height)
{
	SDL_SetWindowSize(window.get(), width, height);
	this->width = width;
	this->height = height;
}

void Window::setMode(WindowMode mode)
{
	auto flags = 0;

	if (mode == WindowMode::Fullscreen)
	{
		flags = SDL_WINDOW_FULLSCREEN;
	}
	else if (mode == WindowMode::Borderless)
	{
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}

	if (SDL_SetWindowFullscreen(window.get(), flags) != 0)
	{
		throw new std::runtime_error("Failed to change window mode, encountered error: " + std::string(SDL_GetError()));
	}

	this->mode = mode;
}

void Window::setShowMouseCursor(bool show)
{
	if (SDL_SetRelativeMouseMode((show ? SDL_FALSE : SDL_TRUE)) < 0)
	{
		throw std::runtime_error("Mouse does not support relative mode, encountered error: " + std::string(SDL_GetError()));
	}

	SDL_WarpMouseInWindow(window.get(), width / 2, height / 2);
}

void Window::showMessageBox(const std::string &title, const std::string &message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), nullptr);
}