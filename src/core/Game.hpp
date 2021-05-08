#pragma once

#include "Camera.hpp"
#include "renderer/Renderer.hpp"

class Game
{
private:
  std::shared_ptr<Window> window;
  std::shared_ptr<Input> input;
  std::shared_ptr<Camera> camera;
  std::shared_ptr<Renderer> renderer;
  std::shared_ptr<Model> oldManModel, weaponModel;

public:
  Game();

  bool update(float delta);
};