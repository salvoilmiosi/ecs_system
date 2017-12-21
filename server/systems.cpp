#include "systems.h"

#include <iostream>

#include "main.h"

void print_func(ecs::entity_id me, printable &p, position &pos) {
	std::cout << "Entity " << p.name << "(" << me << "), Position (" << pos.x << ", " << pos.y << ")" << std::endl;
};

void move_func(ecs::entity_id, position &pos, velocity &vel) {
	pos.x += vel.x;
	pos.y += vel.y;
};

void accelerate_func(ecs::entity_id, velocity &vel, acceleration &acc) {
	vel.x += acc.x;
	vel.y += acc.y;
};

void shrink_func(ecs::entity_id, scale &sca, shrinking &shr) {
	sca.value *= shr.value;
};

void health_tick_func(ecs::entity_id me, health &hp) {
	--hp.value;
	if (hp.value <= 0) {
		server::wld.removeEntity(me);
	}
}
