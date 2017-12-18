#include <SDL2/SDL.h>
#include <fstream>

#include "ecs.h"

#include "systems.h"

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;
static const int FPS = 60;

static SDL_Window *window;

SDL_Renderer *renderer;

ecs::world<MyComponents, MAX_ENTITIES> wld;

namespace {

auto on_tick_systems = std::make_tuple(
	ecs::system<position, generator>(particle_generator_func),
	ecs::system<printable, position>(print_func),
	ecs::system<position, velocity>(move_func),
	ecs::system<velocity, acceleration>(accelerate_func),
	ecs::system<scale, shrinking>(shrink_func),
	ecs::system<health>(health_tick_func)
);

auto on_draw_systems = std::make_tuple(
	ecs::system<sprite, position, scale>(draw_func)
);

bool initSDL() {
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

inline void executeAll(auto &systems) {
	mpl::for_each_in_tuple(systems, [](auto &x) {
		x.execute(wld);
	});
}

void init() {
	wld.createEntity(position(SCREEN_W / 2.0, SCREEN_H / 2.0), generator(20));
	wld.updateEntities();
}

void tick() {
	executeAll(on_tick_systems);
	wld.updateEntities();
}

void render() {
	executeAll(on_draw_systems);
	SDL_RenderPresent(renderer);
}

void clickedMouse() {
	std::ofstream out_file("data", std::ios::out | std::ios::binary);
	wld.logEntities(out_file);
}

}

int main (int argc, char** argv) {
	if (!initSDL()) return 1;

	SDL_Event event;

	init();

	bool quit = false;
	while(!quit) {
		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		tick();
		render();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				clickedMouse();
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