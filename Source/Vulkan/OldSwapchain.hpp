#pragma once

#include "..\Window.hpp"

class Swapchain
{
private:
	bool checkSurfaceCapabilities();
	bool selectSurfaceFormat();
	bool selectSurfacePresentMode();
	bool getImages();
	bool createImageViews();
	bool createFramebuffers(const Window *window);
	bool createCommandBuffers(const Window *window);

public:
	~Swapchain();

	bool create(const Window *window);
	bool finalize(const Window *window);
};