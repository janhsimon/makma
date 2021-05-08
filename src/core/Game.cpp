#include "Game.hpp"
#include "renderer/Settings.hpp"

#include <chrono>
#include <sstream>

Game::Game()
{
  window = std::make_shared<Window>(Settings::windowWidth, Settings::windowHeight,
                                    static_cast<WindowMode>(Settings::windowMode));
  input = std::make_shared<Input>(window);
  camera = std::make_shared<Camera>(window, input, glm::vec3(500.0f, 165.0f, 0.0f), glm::vec3(0.0f, -90.0f, 0.0f),
                                    75.0f, 0.1f, 3000.0f, 5.0f);
  renderer = std::make_shared<Renderer>(window, input, camera);

  weaponModel = renderer->loadModel("models/Machinegun/", "Machinegun.fbx");
  weaponModel->scale = glm::vec3(0.1f);

  renderer->loadModel("models/Sponza/", "Sponza.fbx");
  // auto sanMiguelModel = renderer->loadModel("models/SanMiguel/", "san-miguel-low-poly.obj");
  // sanMiguelModel->scale = glm::vec3(100.0f);

  oldManModel = renderer->loadModel("models/OldMan/", "OldMan.fbx");
  oldManModel->position = glm::vec3(0.0f, -2.0f, 0.0f);

  // light arrays
  for (int i = 0; i < 7; ++i)
  {
    // left lane
    if (i != 2 && i != 3 && i != 4)
    {
      auto lightModel = renderer->loadModel("models/Light/", "Light.fbx");
      lightModel->position = glm::vec3(1100.0f - i * 385.0f, 230.0f, 574.0f);
      lightModel->setYaw(180.0f);
      renderer->loadPointLight(glm::vec3(1100.0f - i * 385.0f, 230.0f, 574.0f - 140.0f), glm::vec3(0.45f, 0.6f, 1.0f),
                               330.0f, 4.0f);
    }

    // right lane
    if (i != 1 && i != 3 && i != 5)
    {
      auto lightModel = renderer->loadModel("models/Light/", "Light.fbx");
      lightModel->position = glm::vec3(1100.0f - i * 385.0f, 230.0f, -644.0f);
      renderer->loadPointLight(glm::vec3(1100.0f - i * 385.0f, 230.0f, -644.0f + 140.0f), glm::vec3(0.45f, 0.6f, 1.0f),
                               330.0f, 4.0f);
    }
  }

  /*
  // and even more lights (far away) to increase the size of the uniform buffer without affecting regular performance,
  giving me more diverse results on this machine for (int i = 0; i < 7 * 10000; ++i)
  {
    renderer->loadPointLight(glm::vec3(1100.0f - i * 385.0f * (1.0f / 10000), 630.0f, 574.0f - 140.0f + 9000.0f),
  glm::vec3(0.45f, 0.6f, 1.0f), 330.0f, 0.1f); renderer->loadPointLight(glm::vec3(1100.0f - i * 385.0f * (1.0f / 10000),
  630.0f, -644.0f + 140.0f + 9000.0f), glm::vec3(0.45f, 0.6f, 1.0f), 330.0f, 0.1f);
  }
  */

  /*
  auto tankModel = renderer->loadModel("models/HeavyTank/", "HeavyTank.fbx");
  tankModel->position += tankModel->getUp() * 115.0f;
  tankModel->position -= tankModel->getRight() * 1000.0f;
  tankModel->position -= tankModel->getForward() * 15.0f;
  tankModel->scale = glm::vec3(400.0f);
  tankModel->setYaw(115.0f);
  tankModel->recalculateAxesFromAngles();
  */

  renderer->loadDirectionalLight(glm::vec3(0.0f, 50.0f, 0.0f), glm::vec3(73.0f, -76.0f, -11.5f),
                                 glm::vec3(1.0f, 0.85f, 0.7f), 1.0f, true);
  renderer->loadDirectionalLight(glm::vec3(100.0f, 50.0f, 0.0f), glm::vec3(0.0f, -30.0f, -30.0f),
                                 glm::vec3(0.6f, 0.4f, 0.3f), 0.45f);
  renderer->loadDirectionalLight(glm::vec3(-100.0f, 50.0f, 0.0f), glm::vec3(-125.0f, 30.0f, 170.0f),
                                 glm::vec3(0.7f, 0.4f, 0.1f), 0.45f);

  renderer->loadPointLight(glm::vec3(1000.0f, 50.0f, 0.0f), glm::vec3(1.0f), 500.0f, 2.0f);
  renderer->loadPointLight(glm::vec3(-1200.0f, 50.0f, 0.0f), glm::vec3(1.0f), 500.0f, 2.0f);

  renderer->loadSpotLight(glm::vec3(400.0f, 50.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(1.0f), 1000.0f, 2.0f,
                          55.0f);

  renderer->finalize();

  std::chrono::high_resolution_clock timer;
  long long frameCount = 1;
  long long frameTime = 0;
  long long averageFrameTime = 0;

  while (true)
  {
    auto startTime = timer.now();

    bool done = false;
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
      {
        done = true;
        break;
      }
      else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESTORED)
      // if the window gets restored (after having been minimized)
      {
        // recreate the swapchain
        renderer->finalize();
      }
      else if (event.type == SDL_MOUSEMOTION)
      {
        input->sendMouseMoveEvent(event);
      }
      else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP)
      {
        input->sendMouseButtonEvent(event);
      }
      else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
      {
        input->sendKeyboardEvent(event);
      }
    }

    if (done)
    {
      break;
    }

    auto applyChanges = update(frameTime / 1000.0f);
    renderer->render();

    auto stopTime = timer.now();
    frameTime = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count();

    if (frameCount < 1)
    // if our frame counter wraps around
    {
      // reset
      frameCount = 1;
      averageFrameTime = 0;
    }

    averageFrameTime += (frameTime - averageFrameTime) / frameCount;
    frameCount++;

    if (applyChanges)
    {
      renderer->waitQueueIdle();
      renderer->finalize();
      renderer->waitQueueIdle();
    }
  }

  renderer->waitQueueIdle();
}

bool Game::update(float delta)
{
  bool applyChanges = renderer->updateUI(delta);

  camera->update(delta);
  input->resetMouseMovement();

  oldManModel->setYaw(oldManModel->getYaw() + delta * 0.1f);
  oldManModel->recalculateAxesFromAngles();

  glm::vec3 weaponTargetPosition =
    camera->position + camera->getForward() * 16.0f - camera->getUp() * 9.0f - camera->getRight() * 3.5f;
  if (input->rightMouseButtonPressed)
  {
    weaponTargetPosition = camera->position + camera->getForward() * 12.0f - camera->getUp() * 8.3f;
  }
  // weaponModel->position = glm::mix(weaponModel->position, weaponTargetPosition, delta *
  // (input->rightMouseButtonPressed ? 0.025f : 0.09f));*/
  weaponModel->position = weaponTargetPosition;

  weaponModel->setYaw(camera->getYaw());
  weaponModel->setPitch(camera->getPitch() - 90.0f);
  weaponModel->setRoll(camera->getRoll());
  weaponModel->recalculateAxesFromAngles();

  renderer->updateBuffers();

  return applyChanges;
}

int main(int argc, char* argv[])
{
  try
  {
    std::make_unique<Game>();
  }
  catch (std::exception& error)
  {
    Window::showMessageBox("Error", error.what());
  }
  catch (...)
  {
    Window::showMessageBox("Error", "Unknown error.");
  }

  return 0;
}
