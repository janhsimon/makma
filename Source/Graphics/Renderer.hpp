#pragma once

#include "Semaphores.hpp"
#include "Swapchain.hpp"
#include "..\Logic\Camera.hpp"

class Renderer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;
	std::unique_ptr<Swapchain> swapchain;
	std::shared_ptr<Buffers> buffers;
	std::shared_ptr<std::vector<Model*>> models;
	std::shared_ptr<Descriptor> descriptor;
	std::shared_ptr<Pipeline> pipeline;
	std::unique_ptr<Semaphores> semaphores;

	std::shared_ptr<Camera> camera;

public:
	Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Camera> camera);

	void update(float delta);
	void render();
	void waitForIdle();
};