#include <SDL2/SDL_ttf.h>

#include "game/game_client.h"
#include "game/game_server.h"

#include "timer.h"

namespace {

const char *TITLE = "Sistema ECS";

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

	SDL_Window *window = SDL_CreateWindow(TITLE,
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

void cleanupSDL() {
	SDL_DestroyRenderer(renderer);
	TTF_Quit();
	SDLNet_Quit();
	SDL_Quit();
}

void parseCommand(const std::string &cmd) {
	bool serv_cmd = server.command(cmd);
	bool client_cmd = client.command(cmd);
	if (!serv_cmd && !client_cmd) {
		console_dev.addLine(console::COLOR_ERROR, console::format(cmd, " is not a valid command."));
	}
}

}

int main (int argc, char** argv) {
	if (!initSDL()) {
		return 1;
	}

	timer fps;

	client.start();

	SDL_Event event;
	while (client.is_open()) {
		fps.start();

		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		if (server.is_open()) {
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

	client.close();
	server.close();

	cleanupSDL();
	return 0;
}
