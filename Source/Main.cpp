#include "Window.hpp"
#include "Vulkan\Renderer.hpp"

int main(int argc, char *argv[])
{
	try
	{
		Window window;
		if (!window.create(1280, 720)) return -1;

		Renderer renderer;
		renderer.create(&window);

		while (true)
		{
			bool done = false;

			SDL_Event event;
			while (SDL_PollEvent(&event))
			{
				if (event.type == SDL_QUIT || (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_ESCAPE))
				{
					done = true;
					break;
				}

				renderer.render();
			}

			if (done)
			{
				break;
			}
		}
		
		renderer.waitForIdle();
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