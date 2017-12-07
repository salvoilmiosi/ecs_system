#include <SDL2/SDL.h>

#include "ecs.h"

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;
static const int FPS = 60;

static SDL_Window *window;
SDL_Renderer *renderer;

bool init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		return false;

	window = SDL_CreateWindow("Sistema ECS",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
	if (window == NULL)
		return false;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == NULL)
		return false;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	return true;
}

void cleanUp() {
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

int main (int argc, char** argv) {
	if (!init()) return 1;

	ecs::createEntity(ecs::position(SCREEN_W / 2.0, SCREEN_H / 2.0), ecs::generator(20));

	SDL_Event event;

	bool quit = false;
	while(!quit) {
		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		ecs::executeAllSystems();

		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}

		SDL_Delay(1000 / FPS);
	}

	cleanUp();
	return 0;
}