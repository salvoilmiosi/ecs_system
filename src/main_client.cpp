#include <SDL2/SDL_ttf.h>

#include "game/game_client.h"
#include "game/game_server.h"

#include "timer.h"

namespace {

const char *TITLE_SERVER = "Sistema ECS - Server";
const char *TITLE_CLIENT = "Sistema ECS - Client";

const char *USER_NAME = "User";

SDL_Renderer *renderer;

void parseCommand(const std::string &cmd);

console::console_ui console_dev(parseCommand, console::CONSOLE_DEV);

game::game_server server(console_dev);
game::game_client client(console_dev);

bool initSDL() {
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1)
		return false;

	if (SDLNet_Init() == -1)
		return false;

	if (TTF_Init() == -1)
		return false;

	SDL_StopTextInput();

	return true;
}

void cleanupSDL() {
	SDL_DestroyRenderer(renderer);
	TTF_Quit();
	SDLNet_Quit();
	SDL_Quit();
}

bool createWindow(const char *title) {
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

void parseCommand(const std::string &cmd) {
	if (!client.command(cmd) && !server.command(cmd)) {
		console_dev.addLine(console::COLOR_ERROR, console::format(cmd, " is not a valid command."));
	}
}

}

int main (int argc, char** argv) {
	if (!initSDL()) {
		return 1;
	}

	bool listenserver = false;

	if (argc == 1) {
		if (server.open()) {
			listenserver = true;
		}
	}

	const char *addr_str = (!listenserver && argc > 1) ? argv[1] : "localhost";

	IPaddress addr;
	if (SDLNet_ResolveHost(&addr, addr_str, net::PORT)) {
		std::cerr << "Could not resolve " << addr_str << std::endl;
		return 2;
	}

	if (! client.connect(addr, USER_NAME)) {
		return 3;
	}
	
	if (! createWindow(listenserver ? TITLE_SERVER : TITLE_CLIENT)) {
		return 4;
	}

	timer fps;

	if (listenserver) {
		server.start();
	}

	client.start();

	SDL_Event event;
	while (client.is_open()) {
		fps.start();

		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		if (listenserver) {
			server.tick();
			server.broadcast();
		}

		client.listen();
		client.tick();
		client.render(renderer);

		console_dev.render(renderer);

		SDL_RenderPresent(renderer);

		while (SDL_PollEvent(&event)) {
			client.handleEvent(event);
		}

		if (fps.get_ticks() < 1000 / net::TICKRATE) {
			SDL_Delay(1000 / net::TICKRATE - fps.get_ticks());
		}
	}

	console_dev.addLine(console::COLOR_LOG, "Disconnected.");

	client.close();
	server.close();

	cleanupSDL();
	return 0;
}
