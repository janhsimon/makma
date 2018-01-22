#pragma once

#include "Renderer/Renderer.hpp"
#include "Camera.hpp"

class Game
{
private:
	std::shared_ptr<Window> window;
	std::shared_ptr<Input> input;
	std::shared_ptr<Camera> camera;
	std::shared_ptr<Renderer> renderer;
	std::shared_ptr<Model> sponzaModel, oldManModel, weaponModel, tankModel;
	std::shared_ptr<Light> pointLight1, pointLight2;

public:
	Game();

	void update(float delta);
};