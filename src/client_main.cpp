#include "systems.h"
#include "client.h"
#include "timer.h"
#include "server_game.h"

#include <memory>
#include <string>

namespace {

constexpr int SCREEN_W = 1024;
constexpr int SCREEN_H = 768;

SDL_Renderer *renderer;

ecs::world_in<MyComponents> wld;

socket::client_socket sock;

void particle_generator_func(ecs::entity_id, position &pos, generator &gen) {
	for (int i=0; i<gen.particles_per_tick; ++i) {
		Uint8 r = rand() % 0xff;
		Uint8 g = rand() % 0xff;
		Uint8 b = rand() % 0xff;
		Uint8 a = rand() % 0xff;
		sprite sprite_random((r << (8 * 3)) | (g << (8 * 2)) | (b << (8 * 1)) | (a << (8 * 0)));
		
		position position_random(
			pos.x + ((float) rand() / RAND_MAX - 0.5f) * 100.f,
			pos.y + ((float) rand() / RAND_MAX - 0.5f) * 100.f);

		velocity velocity_random(
			((float) rand() / RAND_MAX - 0.5f) * 8.f,
			((float) rand() / RAND_MAX - 0.5f) * 8.f);

		acceleration acceleration_random(
			((float) rand() / RAND_MAX - 0.5f) * 0.2f,
			((float) rand() / RAND_MAX - 0.5f) * 0.2f);

		try {
			wld.createEntity(position_random, sprite_random, velocity_random, acceleration_random,
				scale(rand() % 15 + 25.f), shrinking(0.983f), health(rand() % 100 + 50));
			// will create on average 2000 entities
		} catch (std::out_of_range) {
			// out of memory
			return;
		}
	}
}

void draw_func(ecs::entity_id, sprite &spr, position &pos, scale &s) {
	SDL_Rect rect {(int)(pos.x - s.value * 0.5f), (int)(pos.y - s.value * 0.5f), (int)s.value, (int)s.value};
	Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
	Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_RenderFillRect(renderer, &rect);
}

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

static void readServerMessage(packet_data_in &in) {
	std::string msg = readString(in);
	if (msg == "quit") {
		sock.close("Server has quit");
	}
}

static void readPackets() {
	sock.forEachPacket([](auto &x) {
		packet_data_in pdi(x);
		socket::packet_type type = static_cast<socket::packet_type>(readByte(pdi));
		try {
			switch (type) {
			case socket::PACKET_EDITLOG:
				wld.readLog(pdi);
				break;
			case socket::PACKET_SERVERMSG:
				readServerMessage(pdi);
				break;
			case socket::PACKET_NONE:
			default:
				break;
			}
		} catch (std::invalid_argument &err) {
			std::cerr << "Broken packet: " << err.what() << std::endl;
		}
	});

	wld.applyEdits();
}

static void tick() {
	wld.executeSystem<position, generator>(particle_generator_func);
	wld.executeSystem<position, velocity>(move_func);
	wld.executeSystem<velocity, acceleration>(accelerate_func);
	wld.executeSystem<scale, shrinking>(shrink_func);
	wld.executeSystem<health>(health_tick_func);
	wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
		if (hp.value <= 0) {
			wld.removeEntity(me);
		}
	});

	wld.updateEntities();

	readPackets();
}

static void render() {
	wld.executeSystem<sprite, position, scale>(draw_func);

	SDL_RenderPresent(renderer);
}

}

int main (int argc, char** argv) {
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) == -1)
		return 1;

	if (SDLNet_Init() == -1)
		return 1;

	std::shared_ptr<server::game> listenserver;

	if (argc == 1) {
		listenserver = std::make_unique<server::game>();
		if (!listenserver->open()) {
			listenserver = nullptr;
		}
	}

	const char *addr_str = (!listenserver && argc > 1) ? argv[1] : "localhost";

	IPaddress addr;
	if (SDLNet_ResolveHost(&addr, addr_str, socket::PORT)) {
		socket::log("Could not resolve %s\n", addr_str);
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

		if(listenserver) {
			listenserver->tick();
		}

		tick();
		render();

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				sock.disconnect();
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEBUTTONUP:
				sock.sendInputCommand(userinput::handleEvent(event));
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

		if (fps.get_ticks() < 1000 / socket::TICKRATE) {
			SDL_Delay(1000 / socket::TICKRATE - fps.get_ticks());
		}
	}

	cleanUp();
	return 0;
}
