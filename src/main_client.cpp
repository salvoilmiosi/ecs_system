#include <memory>
#include <string>
#include <sstream>

#include "game/game_client.h"
#include "game/game_server.h"

#include "timer.h"

namespace {

constexpr int SCREEN_W = 1024;
constexpr int SCREEN_H = 768;

SDL_Renderer *renderer;

bool initSDL(const char *title) {
	SDL_Window *window = SDL_CreateWindow(title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
	if (window == nullptr)
		return false;

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
		return false;

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	return true;
}

void cleanUp() {
	SDL_DestroyRenderer(renderer);
	SDLNet_Quit();
	SDL_Quit();
}

}

int main (int argc, char** argv) {
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1)
		return 1;

	if (SDLNet_Init() == -1)
		return 1;

	game::game_server listenserver;

	if (argc == 1) {
		listenserver.open();
	}

	const char *addr_str = (!listenserver.is_open() && argc > 1) ? argv[1] : "localhost";

	IPaddress addr;
	if (SDLNet_ResolveHost(&addr, addr_str, net::PORT)) {
		std::cerr << "Could not resolve " << addr_str << std::endl;
		return 2;
	}

	game::game_client client;

	if (! client.connect(addr)) {
		return 3;
	}
	
	if (! initSDL(listenserver.is_open() ? "Sistema ECS - Server" : "Sistema ECS - Client")) {
		return 4;
	}

	timer fps;

	if (listenserver.is_open()) {
		listenserver.start();
	}

	client.start();

	SDL_Event event;
	while (client.is_open()) {
		fps.start();

		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		if (listenserver.is_open()) {
			listenserver.tick();
			listenserver.broadcast();
		}

		client.listen();
		client.tick();
		client.render(renderer);

		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			client.handleEvent(event);
		}

		if (fps.get_ticks() < 1000 / net::TICKRATE) {
			SDL_Delay(1000 / net::TICKRATE - fps.get_ticks());
		}
	}

	console::addLine("Disconnected.");

	client.close();
	listenserver.close();

	cleanUp();
	return 0;
}
