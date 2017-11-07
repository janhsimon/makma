#include "Window.hpp"
#include "Logic\Camera.hpp"
#include "Logic\Input.hpp"
#include "Vulkan\Renderer.hpp"

#include <chrono>

int main(int argc, char *argv[])
{
	try
	{
		auto window = std::make_shared<Window>(1280, 720);
		auto input = std::make_shared<Input>(window);
		auto camera = std::make_shared<Camera>(glm::vec3(0.0f, 50.0f, 0.0f), input);
		auto renderer = std::make_unique<Renderer>(window, camera);

		std::chrono::high_resolution_clock timer;
		long long frameCount = 1;
		long long averageFrameTime = 0;
		long long frameTime = 0;

		while (true)
		{
			auto startTime = timer.now();

			bool done = false;
			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
				{
					input->sendKeyboardEvent(event);
				}
				else if (event.type == SDL_MOUSEMOTION)
				{
					input->sendMouseMoveEvent(event);
				}
				else if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
				{
					done = true;
					break;
				}
			}

			if (done)
			{
				break;
			}

			camera->update(frameTime / 1000.0f);
			input->resetMouseMovement();
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
			window->setTitle("Frametime: " + std::to_string(averageFrameTime) + " microsecs");
			frameCount++;
		}
		
		renderer->waitForIdle();
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