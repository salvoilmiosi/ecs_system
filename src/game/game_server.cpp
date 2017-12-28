#include "game_server.h"

namespace game {

void game_server::tick() {
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

	packet_writer packet;
	writeByte(packet, net::PACKET_EDITLOG);
	wld.flushLog(packet);

	sock.sendAll(packet.data());
}

}