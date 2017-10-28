#include "Window.hpp"
#include "Vulkan\Renderer.hpp"

#include <chrono>

int main(int argc, char *argv[])
{
	try
	{
		auto window = std::make_shared<Window>(1280, 720);
		auto renderer = std::make_unique<Renderer>(window);

		std::chrono::high_resolution_clock timer;
		long long frameCount = 1;
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
			}

			if (done)
			{
				break;
			}

			renderer->update();
			renderer->render();

			auto stopTime = timer.now();
			auto frameTime = std::chrono::duration_cast<std::chrono::microseconds>(stopTime - startTime).count();

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