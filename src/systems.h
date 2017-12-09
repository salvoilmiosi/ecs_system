#include "ecs.h"

#include <iostream>

#include "components.h"

static const int MAX_ENTITIES = ecs::MAX_ENTITIES_DEFAULT;

void print_func(ecs::entity&, printable&, position&);
void move_func(ecs::entity&, position&, velocity&);
void accelerate_func(ecs::entity&, velocity&, acceleration&);
void shrink_func(ecs::entity&, scale&, shrinking&);
void draw_func(ecs::entity&, sprite&, position&, scale&);
void health_tick_func(ecs::entity&, health&);
void particle_generator_func(ecs::entity&, position&, generator&);