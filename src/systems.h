#include "ecs.h"

#include <iostream>

#include "components.h"

static const int MAX_ENTITIES = 200;

void print_func(ecs::entity_id, printable&, position&);
void move_func(ecs::entity_id, position&, velocity&);
void accelerate_func(ecs::entity_id, velocity&, acceleration&);
void shrink_func(ecs::entity_id, scale&, shrinking&);
void draw_func(ecs::entity_id, sprite&, position&, scale&);
void health_tick_func(ecs::entity_id, health&);
void particle_generator_func(ecs::entity_id, position&, generator&);