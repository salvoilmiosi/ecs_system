#ifndef __SERVER_GAME_H__
#define __SERVER_GAME_H__

#include "ecs/world_io.h"

#include "net/server.h"

namespace game {

class game_server {
public:
	game_server(console::console &console_dev) : sock(wld, console_dev) {}

	~game_server() {
		close();
	}

	bool open() {
		return sock.open();
	}

	void start();

	void tick();

	void broadcast();

	bool command(const std::string &full_cmd);

	bool is_open() {
		return sock.is_open();
	}

	void close() {
		sock.close();
		wld.clear();
	}

private:
	ecs::world_out<MyComponents> wld;

	net::server_socket sock;
};

}

#endif // __SERVER_GAME_H__