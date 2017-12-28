#include "ecs/world.h"

#include "components.h"

namespace game {

void move_func(ecs::entity_id, position &pos, velocity &vel);

void accelerate_func(ecs::entity_id, velocity &vel, acceleration &acc);

void shrink_func(ecs::entity_id, scale &sca, shrinking &shr);

void health_tick_func(ecs::entity_id, health &hp);

void particle_generator_func(ecs::entity_id, position &pos, generator &gen);

}