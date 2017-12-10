#pragma once

#include "Model.hpp"
#include "Semaphores.hpp"
#include "GeometryPass\GeometryBuffer.hpp"
#include "LightingPass\Swapchain.hpp"
#include "..\Logic\Camera.hpp"


class Renderer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Context> context;
	std::shared_ptr<Buffers> buffers;
	std::vector<Model*> models;
	std::shared_ptr<Descriptor> descriptor;
	std::shared_ptr<GeometryBuffer> geometryBuffer;
	std::shared_ptr<GeometryPipeline> geometryPipeline;
	std::unique_ptr<Swapchain> swapchain;
	std::shared_ptr<LightingPipeline> lightingPipeline;
	std::unique_ptr<Semaphores> semaphores;
	
public:
	Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Camera> camera); 
	~Renderer() { for (size_t i = 0; i < models.size(); ++i) delete models[i]; }

	void update(float delta);
	void render();
	void waitForIdle();
};