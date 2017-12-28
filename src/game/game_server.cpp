#include "game_server.h"

namespace game {

void game_server::tick() {
	wld.executeSystem<position, velocity>([&](ecs::entity_id id, position &pos, velocity &vel) {
		pos.value.x += vel.value.x;
		pos.value.y += vel.value.y;
	});
	wld.executeSystem<velocity, acceleration>([&](ecs::entity_id id, velocity &vel, acceleration &acc) {
		vel.value.x += acc.value.x;
		vel.value.y += acc.value.y;
	});
	wld.executeSystem<scale, shrinking>([&](ecs::entity_id id, scale &sca, shrinking &shr) {
		sca.value *= shr.value;
	});
	wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
		--hp.value;
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