#pragma once

#include "Window.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Input
{
private:
  std::shared_ptr<Window> window;

  unsigned short windowHalfWidth, windowHalfHeight;

public:
  bool forwardKeyPressed;
  bool backKeyPressed;
  bool leftKeyPressed;
  bool rightKeyPressed;
  bool upKeyPressed;
  bool downKeyPressed;
  bool crouchKeyPressed;
  bool flyKeyPressed;
  bool showCursorKeyPressed;
  bool showControlsWindowKeyPressed;
  bool showBenchmarkWindowKeyPressed;
  bool showLightEditorKeyPressed;
  bool showGraphsKeyPressed;
  bool showResultsWindowKeyPressed;
  bool leftMouseButtonPressed;
  bool rightMouseButtonPressed;

  glm::ivec2 mouseDelta;

  Input(const std::shared_ptr<Window> window);

  void sendMouseMoveEvent(const SDL_Event& event);
  void sendMouseButtonEvent(const SDL_Event& event);
  void sendKeyboardEvent(const SDL_Event& event);

  void resetMouseMovement();

  glm::vec2 getMousePosition() const
  {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return glm::vec2(x, y);
  }
};