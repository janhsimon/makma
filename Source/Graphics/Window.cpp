#include "Window.hpp"

#include <vector>

SDL_Window *Window::createWindow(unsigned short width, unsigned short height)
{
	SDL_Window *window;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		throw std::runtime_error("Failed to initialize SDL video subsystem, encountered error: " + std::string(SDL_GetError()));
	}

	window = SDL_CreateWindow("Makma", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);

	if (!window)
	{
		throw std::runtime_error("Failed to create window, encountered error: " + std::string(SDL_GetError()));
	}

	return window;
}

Window::Window(unsigned short width, unsigned short height)
{
	window = std::unique_ptr<SDL_Window, decltype(windowDeleter)>(createWindow(width, height), windowDeleter);

	SDL_ShowCursor(false);

	windowWidth = width;
	windowHeight = height;
}

void Window::setTitle(const std::string &title)
{
	SDL_SetWindowTitle(window.get(), title.c_str());
}

void Window::showMessageBox(const std::string &title, const std::string &message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), nullptr);
}