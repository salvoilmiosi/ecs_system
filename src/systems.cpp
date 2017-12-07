#include "ecs.h"

#include <SDL2/SDL.h>
#include <iostream>

#include <cstdlib>

using namespace components;

namespace systems {
	void print(reflect &me, printable &p, position &pos) {
		std::cout << "Entity " << p.name << "(" << me.id << "), Position (" << pos.x << ", " << pos.y << ")" << std::endl;
	}

	void draw(sprite &spr, position &pos, scale &s) {
		// TODO SDL draw
	}

	void tick(reflect &me, health &hp) {
		--hp.value;
		if (hp.value <= 0) {
			std::cout << "Entity " << me.id << " is dead" << std::endl;
			ecs::removeEntity(me.id);
		}
	}

	velocity velocity_random() {
		velocity ret;
		ret.x = (float) rand() / RAND_MAX;
		ret.y = (float) rand() / RAND_MAX;
		return ret;
	}

	void generate(position &pos, generator &gen) {
		for (int i=0; i<gen.particles_per_tick; ++i) {
			ecs::entity_id ent = ecs::createEntity(pos, printable("Particle"), sprite(0xff0000ff), velocity_random(), scale(10.f), health());
			ecs::addComponent(ent, reflect(ent));
		}
	}
}