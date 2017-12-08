#include "ecs.h"

#include <iostream>

#include "components.h"

extern ecs::system<printable, position> print;
extern ecs::system<position, velocity> move;
extern ecs::system<velocity, acceleration> accelerate;
extern ecs::system<scale, shrinking> shrinker;

void draw_func(ecs::entity&, sprite&, position&, scale&);
void health_tick_func(ecs::entity&, health&);
void particle_generator_func(ecs::entity&, position&, generator&);