#include "systems.h"

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

void health_tick_func(ecs::entity_id, health &hp) {
	--hp.value;
};