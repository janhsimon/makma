#pragma once

#include <SDL/SDL.h>

#include <functional>
#include <memory>
#include <string>

enum WindowMode
{
	Windowed,
	Fullscreen,
	Borderless
};

class Window
{
private:
	static SDL_Window *createWindow(unsigned short width, unsigned short height, WindowMode mode);
	std::function<void(SDL_Window*)> windowDeleter = [this](SDL_Window *window) { SDL_DestroyWindow(window); SDL_Quit(); };
	std::unique_ptr<SDL_Window, decltype(windowDeleter)> window;

	unsigned short width, height;
	WindowMode mode;

public:
	Window(unsigned short width, unsigned short height, WindowMode mode);

	void setSize(unsigned short width, unsigned short height);

	SDL_Window *getWindow() const { return window.get(); }

	unsigned short getWidth() const { return width; }
	unsigned short getHeight() const { return height; }

	WindowMode getMode() const { return mode; }
	void setMode(WindowMode mode);

	bool getShowMouseCursor() const { return !SDL_GetRelativeMouseMode(); }
	void setShowMouseCursor(bool show);

	static void showMessageBox(const std::string &title, const std::string &message);
};