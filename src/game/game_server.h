#ifndef __SERVER_GAME_H__
#define __SERVER_GAME_H__

#include "ecs/world_io.h"

#include "net/server.h"

namespace game {

class game_server {
public:
	game_server() : sock(wld) {}

	bool open() {
		return sock.open();
	}

	void start();

	void tick();

	void broadcast();

	void command(std::string cmd) {
		if (cmd == "quit") {
			close();
		}
	}

	bool is_open() {
		return sock.is_open();
	}

	void close() {
		sock.close();
	}

private:
	ecs::world_out<MyComponents> wld;

	net::server_socket sock;
};

}

#endif // __SERVER_GAME_H__