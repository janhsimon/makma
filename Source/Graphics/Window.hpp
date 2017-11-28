#pragma once

#include <SDL.h>

#include <functional>
#include <memory>
#include <string>

class Window
{
private:
	static SDL_Window *createWindow(unsigned short width, unsigned short height, bool fullscreen);
	std::function<void(SDL_Window*)> windowDeleter = [this](SDL_Window *window) { SDL_DestroyWindow(window); SDL_Quit(); };
	std::unique_ptr<SDL_Window, decltype(windowDeleter)> window;

	unsigned short windowWidth, windowHeight;

public:
	Window(unsigned short width, unsigned short height, bool fullscreen);

	void setTitle(const std::string &title);

	SDL_Window *getWindow() const { return window.get(); }

	unsigned short getWidth() const { return windowWidth; }
	unsigned short getHeight() const { return windowHeight; }

	static void showMessageBox(const std::string &title, const std::string &message);
};