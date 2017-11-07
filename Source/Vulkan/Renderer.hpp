#pragma once

#include "Model.hpp"
#include "Semaphores.hpp"
#include "Swapchain.hpp"
#include "Texture.hpp"
#include "..\Logic\Camera.hpp"

class Renderer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Context> context;
	std::unique_ptr<Swapchain> swapchain;
	std::shared_ptr<Buffers> buffers;
	std::unique_ptr<Model> model;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Pipeline> pipeline;
	std::unique_ptr<Semaphores> semaphores;

	std::shared_ptr<Camera> camera;

public:
	Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Camera> camera);

	void render();
	void waitForIdle();
};