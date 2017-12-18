#include "main.h"

#include "systems.h"

namespace client {

SDL_Window *window;

SDL_Renderer *renderer;

ecs::world<MyComponents, MAX_ENTITIES> wld(true);

static auto on_tick_systems = std::make_tuple(
	ecs::system<printable, position>(print_func),
	ecs::system<position, velocity>(move_func),
	ecs::system<velocity, acceleration>(accelerate_func),
	ecs::system<scale, shrinking>(shrink_func),
	ecs::system<health>(health_tick_func)
);

static auto on_draw_systems = std::make_tuple(
	ecs::system<sprite, position, scale>(draw_func)
);

static bool initSDL() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1)
		return false;

	if (SDLNet_Init())
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

static void cleanUp() {
	SDL_DestroyRenderer(renderer);
	SDLNet_Quit();
	SDL_Quit();
}

static inline void executeAll(auto &systems) {
	mpl::for_each_in_tuple(systems, [](auto &x) {
		x.execute(wld);
	});
}

static void init() {
	
}

static void tick() {
	executeAll(on_tick_systems);
	wld.updateEntities();
}

static void render() {
	executeAll(on_draw_systems);
	SDL_RenderPresent(renderer);
}

}

int main (int argc, char** argv) {
	if (!client::initSDL()) return 1;

	SDL_Event event;

	client::init();

	bool quit = false;
	while(!quit) {
		SDL_SetRenderDrawColor(client::renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(client::renderer);

		client::tick();
		client::render();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}

		SDL_Delay(1000 / client::FPS);
	}

	client::cleanUp();
	return 0;
}
