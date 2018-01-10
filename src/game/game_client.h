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
	game_client(console::console_dev &console_dev);
	
	bool connect(IPaddress addr) {
		disconnect(); // If connected already
		return sock.connect(addr, username);
	}

	bool connect(const std::string &addr) {
		disconnect(); // If connected already
		if (! sock.connect(addr, username)) {
			console_dev.addLine(console::COLOR_ERROR, console::format("Could not resolve ", addr));
			return false;
		}
		return true;
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
	console::console_dev &console_dev;
	console::console_chat console_chat;

	bool open = true;

	std::string username;
	
	ecs::world_in<MyComponents> wld;

	ecs::edit_logger<MyComponents> in_logger;

	net::client_socket sock;

	void generateParticles(position &pos);

	void renderSquare(SDL_Renderer *renderer, ecs::entity_id ent, color &col, position &pos, scale &s);

	void renderCircle(SDL_Renderer *renderer, ecs::entity_id ent, color &col, position &pos, scale &s);

	bool collides(ecs::entity_id ball_a, ecs::entity_id ball_b);

	void resolveCollision(ecs::entity_id ball_a, ecs::entity_id ball_b);
};

}

#endif // __GAME_CLIENT_H__