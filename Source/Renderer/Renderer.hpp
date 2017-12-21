#pragma once

#include "Model.hpp"
#include "Semaphores.hpp"
#include "GeometryPass\GeometryBuffer.hpp"
#include "LightingPass\Swapchain.hpp"
#include "..\Camera.hpp"
#include "..\Light.hpp"

class Renderer
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Context> context;
	std::shared_ptr<Buffers> buffers;
	std::vector<std::shared_ptr<Model>> modelList;
	std::vector<std::shared_ptr<Light>> lightList;
	std::shared_ptr<Descriptor> descriptor;
	std::shared_ptr<GeometryBuffer> geometryBuffer;
	std::shared_ptr<GeometryPipeline> geometryPipeline;
	std::unique_ptr<Swapchain> swapchain;
	std::shared_ptr<LightingPipeline> lightingPipeline;
	std::unique_ptr<Semaphores> semaphores;
	
public:
	Renderer(const std::shared_ptr<Window> window, const std::shared_ptr<Input> input, const std::shared_ptr<Camera> camera);
	
	std::shared_ptr<Model> loadModel(const std::string &path, const std::string &filename);
	std::shared_ptr<Light> loadLight(LightType type, const glm::vec3 &position, const glm::vec3 &color = glm::vec3(1.0f), float range = 1000.0f, float intensity = 1.0f, float specularPower = 4.0f);

	void finalize();
	void update();
	void render();
	void waitForIdle();
};