#ifndef __GAME_H__
#define __GAME_H__

#include "systems.h"
#include "server.h"

namespace server {

class game {
public:
	game() : sock(wld) {}

	bool open() {
		return sock.open();
	}

	void tick() {
		wld.executeSystem<printable, position>(print_func);
		wld.executeSystem<position, velocity>(move_func);
		wld.executeSystem<velocity, acceleration>(accelerate_func);
		wld.executeSystem<scale, shrinking>(shrink_func);

		wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
			--hp.value;
			if (hp.value <= 0) {
				wld.removeEntity(me);
			}
		});

		wld.updateEntities();

		packet_data_out packet;
		writeByte(packet, socket::PACKET_EDITLOG);
		wld.flushLog(packet);

		sock.sendAll(packet.data());
	}

	void command(std::string cmd) {
		if (cmd == "quit") {
			sock.sendServerMsg("quit");
			quit = true;
		}
	}

	bool is_open() {
		return !quit;
	}

	void close() {
		sock.close();
	}

private:
	ecs::world_out<MyComponents> wld;

	socket::server_socket sock;

	bool quit = false;
};

}

#endif // __GAME_H__