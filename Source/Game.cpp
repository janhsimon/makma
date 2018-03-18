#include "Game.hpp"

#include <chrono>
#include <sstream>

Game::Game()
{
#ifdef _DEBUG
	window = std::make_shared<Window>(1280, 720, false);
#else
	window = std::make_shared<Window>(1920, 1080, true);
#endif

	input = std::make_shared<Input>(window);
	camera = std::make_shared<Camera>(window, input, glm::vec3(0.0f, 160.0f, 0.0f), 75.0f, 0.1f, 3000.0f, 5.0f);
	renderer = std::make_shared<Renderer>(window, input, camera);

	weaponModel = renderer->loadModel("Models/Machinegun/", "Machinegun.fbx");
	weaponModel->scale = glm::vec3(0.1f);

	sponzaModel = renderer->loadModel("Models/Sponza/", "Sponza.fbx");
	//sponzaModel = renderer->loadModel("Models/SanMiguel/", "san-miguel-low-poly.obj");
	//sponzaModel->scale = glm::vec3(100.0f);

	oldManModel = renderer->loadModel("Models/OldMan/", "OldMan.fbx");

	/*
	tankModel = renderer->loadModel("Models/HeavyTank/", "HeavyTank.fbx");
	tankModel->position += tankModel->getUp() * 115.0f;
	tankModel->position -= tankModel->getRight() * 1000.0f;
	tankModel->position -= tankModel->getForward() * 15.0f;
	tankModel->scale = glm::vec3(400.0f);
	tankModel->setYaw(115.0f);
	tankModel->recalculateAxesFromAngles();
	*/

	renderer->loadLight(LightType::Directional, glm::vec3(-0.4f, -1.0f, 0.1f), glm::vec3(1.0f, 0.85f, 0.7f), 1.0f, 1.0f, true);
	renderer->loadLight(LightType::Directional, glm::vec3(1.0f, -1.0f, -1.0f), glm::vec3(0.6f, 0.4f, 0.3f), 1.0f, 0.35f);
	renderer->loadLight(LightType::Directional, glm::vec3(-1.0f, 1.0f, 1.0f), glm::vec3(0.7f, 0.4f, 0.1f), 1.0f, 0.35f);
	
	pointLight1 = renderer->loadLight(LightType::Point, glm::vec3(1000.0f, 50.0f, 0.0f), glm::vec3(1.0f), 500.0f, 5.0f);
	pointLight2 = renderer->loadLight(LightType::Point, glm::vec3(-1200.0f, 50.0f, 0.0f), glm::vec3(1.0f), 500.0f, 5.0f);

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

		update(frameTime / 1000.0f);
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
	}

	renderer->waitForIdle();
}

void Game::update(float delta)
{
	renderer->getUI()->update(input, camera, delta);
	camera->update(delta);
	input->resetMouseMovement();

	oldManModel->setYaw(oldManModel->getYaw() + delta * 0.1f);
	oldManModel->recalculateAxesFromAngles();

	glm::vec3 weaponTargetPosition = camera->position + camera->getForward() * 16.0f - camera->getUp() * 9.0f - camera->getRight() * 3.5f;
	if (input->rightMouseButtonPressed)
	{
		weaponTargetPosition = camera->position + camera->getForward() * 12.0f - camera->getUp() * 8.3f;
		
	}
	//weaponModel->position = glm::mix(weaponModel->position, weaponTargetPosition, delta * (input->rightMouseButtonPressed ? 0.025f : 0.09f));
	weaponModel->position = weaponTargetPosition;

	weaponModel->setYaw(camera->getYaw());
	weaponModel->setPitch(camera->getPitch() - 90.0f);
	weaponModel->setRoll(camera->getRoll());
	weaponModel->recalculateAxesFromAngles();

	pointLight1->position.y += 0.1f * delta;
	pointLight2->position.y += 0.1f * delta;

	if (pointLight1->position.y > 1500.0f) pointLight1->position.y = 0.0f;
	if (pointLight2->position.y > 1500.0f) pointLight2->position.y = 0.0f;

	renderer->update();
}

int main(int argc, char *argv[])
{
	try
	{
		std::make_unique<Game>();
	}
	catch (std::exception &error)
	{
		Window::showMessageBox("Error", error.what());
	}
	catch (...)
	{
		Window::showMessageBox("Error", "Unknown error.");
	}

	return 0;
}
