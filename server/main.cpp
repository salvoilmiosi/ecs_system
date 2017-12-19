#include "main.h"

#include "systems.h"

#include "socket.h"

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

void handleMouse(IPaddress addr, SDL_MouseButtonEvent mouse) {
	if (mouse.type == SDL_MOUSEBUTTONDOWN) {
		wld.createEntity(position(mouse.x, mouse.y), generator(50), health(1));
	}
}

static void tick() {
	executeAll(on_tick_systems);

	wld.updateEntities();

	packet_data_out packet;
	wld.flushLog(packet);

	sock.sendAll(packet.data());
}

}

int main (int argc, char** argv) {
	if (SDL_Init(SDL_INIT_TIMER) == -1)
		return 1;

	if (SDLNet_Init() == -1)
		return 2;

	server::sock.open();
	server::sock.run();

	bool quit = false;
	while(!quit) {
		server::tick();

		SDL_Delay(1000 / server::FPS);
	}

	server::sock.close();

	SDLNet_Quit();
	
	SDL_Quit();
	return 0;
}