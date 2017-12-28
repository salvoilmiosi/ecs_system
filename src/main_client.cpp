#include <memory>
#include <string>

#include "game/game_client.h"
#include "game/game_server.h"

#include "net/client.h"

#include "timer.h"

namespace {

constexpr int SCREEN_W = 1024;
constexpr int SCREEN_H = 768;

SDL_Renderer *renderer;

game::game_client my_game(renderer);

ecs::edit_logger<MyComponents> input_logger;

net::client_socket sock;

static bool initSDL(const char *title) {
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

static void cleanUp() {
	SDL_DestroyRenderer(renderer);
	SDLNet_Quit();
	SDL_Quit();
}

static void readServerMessage(packet_reader &in) {
	std::string msg = readString(in);
	if (msg == "quit") {
		sock.close("Server has quit");
	}
}

void readPackets() {
	sock.forEachPacket([](auto &x) {
		packet_reader pdi(x);
		net::packet_type type = static_cast<net::packet_type>(readByte(pdi));
		try {
			switch (type) {
			case net::PACKET_EDITLOG:
				input_logger.read(pdi);
				break;
			case net::PACKET_SERVERMSG:
				readServerMessage(pdi);
				break;
			case net::PACKET_NONE:
			default:
				break;
			}
		} catch (std::invalid_argument &err) {
			std::cerr << "Broken packet: " << err.what() << std::endl;
		}
	});

	my_game.applyEdits(input_logger);
}

}

int main (int argc, char** argv) {
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1)
		return 1;

	if (SDLNet_Init() == -1)
		return 1;

	std::shared_ptr<game::game_server> listenserver;

	if (argc == 1) {
		listenserver = std::make_unique<game::game_server>();
		if (!listenserver->open()) {
			listenserver = nullptr;
		}
	}

	const char *addr_str = (!listenserver && argc > 1) ? argv[1] : "localhost";

	IPaddress addr;
	if (SDLNet_ResolveHost(&addr, addr_str, net::PORT)) {
		logger::log("Could not resolve ", addr_str);
		return 2;
	}

	if (! sock.connect(addr)) {
		return 3;
	}
	
	if (! initSDL(listenserver ? "Sistema ECS - Server" : "Sistema ECS - Client")) {
		return 4;
	}

	timer fps;

	SDL_Event event;
	while(sock.is_open()) {
		fps.start();

		SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xff);
		SDL_RenderClear(renderer);

		readPackets();

		if(listenserver) {
			listenserver->tick();
		}

		my_game.tick();

		my_game.render();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				sock.disconnect();
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONUP:
				sock.sendInputCommand(game::userinput::handleEvent(event));
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_SPACE) {
					sock.sendCommand("state");
				}
				break;
			default:
				break;
			}
		}

		if (fps.get_ticks() < 1000 / net::TICKRATE) {
			SDL_Delay(1000 / net::TICKRATE - fps.get_ticks());
		}
	}

	cleanUp();
	return 0;
}
