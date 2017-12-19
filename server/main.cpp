#include "main.h"

#include <thread>

#include "systems.h"
#include "socket.h"
#include "timer.h"

namespace server {

ecs::world_out<MyComponents, MAX_ENTITIES> wld;

socket::server_socket sock;

static auto on_tick_systems = std::make_tuple(
	ecs::system<printable, position>(print_func),
	ecs::system<position, velocity>(move_func),
	ecs::system<velocity, acceleration>(accelerate_func),
	ecs::system<scale, shrinking>(shrink_func),
	ecs::system<health>(health_tick_func)
);

static inline void executeAll(auto &systems) {
	mpl::for_each_in_tuple(systems, [](auto &x) {
		x.execute(wld);
	});
}

static void tick() {
	executeAll(on_tick_systems);

	wld.updateEntities();

	packet_data_out packet;
	wld.flushLog(packet);

	sock.sendAll(packet.data());
}

static std::thread cmdline;

static bool quit = false;

void command(std::string cmd) {
	if (cmd == "quit") {
		quit = true;
	}
}

}

int main (int argc, char** argv) {
	if (SDL_Init(SDL_INIT_TIMER) == -1)
		return 1;

	if (SDLNet_Init() == -1)
		return 2;

	if (!server::sock.open())
		return 3;

	server::cmdline = std::thread([&]() {
		std::string line;
		while (!server::quit) {
			std::getline(std::cin, line);

			server::command(line);
		}
	});

	timer fps;

	while(!server::quit) {
		fps.start();

		server::tick();

		if (fps.get_ticks() < 1000 / server::TICKRATE) {
			SDL_Delay(1000 / server::TICKRATE - fps.get_ticks());
		}
	}

	server::sock.close();
	server::cmdline.join();

	SDLNet_Quit();
	SDL_Quit();
	return 0;
}