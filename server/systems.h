#include "ecs.h"

#include <iostream>

#include "components.h"

static const int MAX_ENTITIES = ecs::MAX_ENTITIES_DEFAULT;

void print_func(ecs::entity_id, printable&, position&);
void move_func(ecs::entity_id, position&, velocity&);
void accelerate_func(ecs::entity_id, velocity&, acceleration&);
void shrink_func(ecs::entity_id, scale&, shrinking&);
void health_tick_func(ecs::entity_id, health&);