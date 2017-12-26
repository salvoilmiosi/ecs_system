#include "ecs.h"

#include "components.h"

void print_func(ecs::entity_id me, printable &p, position &pos);

void move_func(ecs::entity_id, position &pos, velocity &vel);

void accelerate_func(ecs::entity_id, velocity &vel, acceleration &acc);

void shrink_func(ecs::entity_id, scale &sca, shrinking &shr);

void health_tick_func(ecs::entity_id, health &hp);