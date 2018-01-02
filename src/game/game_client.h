#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL.h>

#include "ecs/world_io.h"

#include "console/console_ui.h"

#include "net/client.h"

#include "components.h"

namespace game {

class game_client {
public:
	game_client(console::console_ui &console_dev);
	
	bool connect(IPaddress addr) {
		if (sock.is_open()) {
			disconnect();
		}
		return sock.connect(addr, username);
	}

	bool connect(const std::string &str) {
		IPaddress addr;
		if (SDLNet_ResolveHost(&addr, str.c_str(), net::PORT)) {
			console_dev.addLine(console::COLOR_ERROR, console::format("Could not resolve ", str));
			return false;
		}
		return connect(addr);
	}

	void disconnect() {
		if (!sock.is_open()) return;
		
		console_dev.addLine(console::COLOR_LOG, "Disconnected.");

		sock.disconnect();
		wld.clear();
	}

	void start();

	void tick();

	void listen();

	void render(SDL_Renderer *renderer);
	
	void handleEvent(const SDL_Event &e);

	bool command(const std::string &full_cmd);

	bool is_open() {
		return open;
	}

	void close() {
		disconnect();
		open = false;
	}

private:
	console::console_ui &console_dev;
	console::console_ui console_chat;

	bool open = true;

	std::string username;
	
	ecs::world_in<MyComponents> wld;

	ecs::edit_logger<MyComponents> in_logger;

	net::client_socket sock;

	void generateParticles(position &pos);

	void renderEntity(SDL_Renderer *renderer, sprite &spr, position &pos, scale &s);
};

}

#endif // __GAME_CLIENT_H__