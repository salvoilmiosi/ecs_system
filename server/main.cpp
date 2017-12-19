#include "main.h"

#include "systems.h"

#include "socket.h"

#include <sstream>

namespace server {

SDL_Window *window;

SDL_Renderer *renderer;

ecs::world_out<MyComponents, MAX_ENTITIES> wld;

socket::server_socket sock;

static auto on_tick_systems = std::make_tuple(
	ecs::system<position, generator>(particle_generator_func),
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

	if (SDLNet_Init() == -1)
		return false;

	window = SDL_CreateWindow("Sistema ECS - Server",
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
	sock.close();

	SDLNet_Quit();
	
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

static inline void executeAll(auto &systems) {
	mpl::for_each_in_tuple(systems, [](auto &x) {
		x.execute(wld);
	});
}

static void init() {
	sock.open();
	sock.run();

	wld.createEntity(position(SCREEN_W / 2.0, SCREEN_H / 2.0), generator(10));
	wld.updateEntities();
}

static void broadcast() {
	std::ostringstream oss(std::ios::out | std::ios::binary);

	wld.flushLog(oss);

	std::string packet = oss.str();
	sock.sendAll((Uint8 *)packet.data(), packet.size());
}

static void tick() {
	executeAll(on_tick_systems);
	wld.updateEntities();

	broadcast();
}

static void render() {
	executeAll(on_draw_systems);
	SDL_RenderPresent(renderer);
}


}

int main (int argc, char** argv) {
	if (!server::initSDL()) return 1;

	SDL_Event event;

	server::init();

	bool quit = false;
	while(!quit) {
		SDL_SetRenderDrawColor(server::renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(server::renderer);

		server::tick();
		server::render();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			default:
				break;
			}
		}

		SDL_Delay(1000 / server::FPS);
	}

	server::cleanUp();
	return 0;
}