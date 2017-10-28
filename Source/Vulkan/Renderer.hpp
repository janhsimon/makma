#pragma once

#include "Semaphores.hpp"
#include "Swapchain.hpp"

class Renderer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;
	std::unique_ptr<Swapchain> swapchain;
	std::shared_ptr<Pipeline> pipeline;
	std::shared_ptr<Buffers> buffers;
	std::unique_ptr<Semaphores> semaphores;

public:
	Renderer(const std::shared_ptr<Window> window);

	void update();
	void render();
	void waitForIdle();
};