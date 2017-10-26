#include "Window.hpp"

#include <vector>

Window::~Window()
{
	if (window)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	SDL_Quit();
}

bool Window::create(unsigned short width, unsigned short height)
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		showMessageBox("Error", "Failed to initialize SDL video subsystem, encountered error: " + std::string(SDL_GetError()));
		return false;
	}

	window = SDL_CreateWindow("Makma", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);

	if (!window)
	{
		showMessageBox("Error", "Failed to create window, encountered error: " + std::string(SDL_GetError()));
		return false;
	}

	windowWidth = width;
	windowHeight = height;

	return true;
}

void Window::showMessageBox(const std::string &title, const std::string &message)
{
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title.c_str(), message.c_str(), nullptr);
}

bool Window::showChoiceBox(const std::string &title, const std::string &message, const std::vector<std::string> &buttonCaptions, int &buttonID)
{
	std::vector<SDL_MessageBoxButtonData> buttonsData(buttonCaptions.size());

	for (unsigned short i = 0; i < buttonsData.size(); ++i)
	{
		buttonsData[i] = { 0, i, buttonCaptions[i].c_str() };
	}

	const SDL_MessageBoxData messageBoxData = {	SDL_MessageBoxFlags::SDL_MESSAGEBOX_INFORMATION, nullptr, title.c_str(), message.c_str(), buttonCaptions.size(), buttonsData.data(), nullptr };

	if (SDL_ShowMessageBox(&messageBoxData, &buttonID) < 0)
	{
		showMessageBox("Error", "Failed to display a message box, encountered error: " + std::string(SDL_GetError()));
		return false;
	}

	return true;
}