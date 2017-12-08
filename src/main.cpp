#include <SDL2/SDL.h>

#include "ecs.h"

#include "systems.h"

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;
static const int FPS = 60;

static SDL_Window *window;
SDL_Renderer *renderer;

ecs::manager<MyComponents> mgr;

auto MySystems = std::make_tuple(
	ecs::system<printable, position>(print_func),
	ecs::system<position, velocity>(move_func),
	ecs::system<velocity, acceleration>(accelerate_func),
	ecs::system<scale, shrinking>(shrink_func),
	ecs::system<sprite, position, scale>(draw_func),
	ecs::system<health>(health_tick_func),
	ecs::system<position, generator>(particle_generator_func));

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

void executeAllSystems() {
	ecs::for_each_in_tuple(MySystems,[](auto &x){
		mgr.executeSystem(x);
	});
}

int main (int argc, char** argv) {
	if (!init()) return 1;

	mgr.createEntity(position(SCREEN_W / 2.0, SCREEN_H / 2.0), generator(20));
	mgr.updateEntities();

	SDL_Event event;

	bool quit = false;
	while(!quit) {
		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		executeAllSystems();
		mgr.updateEntities();

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