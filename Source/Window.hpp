#pragma once

#include <SDL.h>

#include <string>
#include <vector>

class Window
{
private:
	SDL_Window *window = nullptr;
	unsigned short windowWidth, windowHeight;

public:
	~Window();

	bool create(unsigned short width, unsigned short height);

	SDL_Window *getWindow() const { return window; }

	unsigned short getWidth() const { return windowWidth; }
	unsigned short getHeight() const { return windowHeight; }

	static void showMessageBox(const std::string &title, const std::string &message);
	static bool showChoiceBox(const std::string &title, const std::string &message, const std::vector<std::string> &buttonCaptions, int &buttonID);
};