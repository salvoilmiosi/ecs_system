#include "ecs.h"

#include <SDL2/SDL.h>
#include <iostream>

#include <cstdlib>

using namespace components;

extern SDL_Renderer *renderer;

namespace systems {
	void print(reflect &me, printable &p, position &pos) {
		std::cout << "Entity " << p.name << "(" << me.id << "), Position (" << pos.x << ", " << pos.y << ")" << std::endl;
	}

	void draw(sprite &spr, position &pos, scale &s) {
		SDL_Rect rect {pos.x, pos.y, s.value, s.value};
		SDL_SetRenderDrawColor(renderer, 0xff, 0x0, 0x0, 0xff);//spr.color);
		SDL_RenderFillRect(renderer, &rect);
	}

	void tick(reflect &me, health &hp) {
		--hp.value;
		if (hp.value <= 0) {
			//std::cout << "Entity " << me.id << " is dead" << std::endl;
			ecs::removeEntity(me.id);
		}
	}

	velocity velocity_random() {
		velocity ret;
		ret.x = ((float) rand() / RAND_MAX - 0.5f) * 5.f;
		ret.y = ((float) rand() / RAND_MAX - 0.5f) * 5.f;
		return ret;
	}

	void generate(position &pos, generator &gen) {
		for (int i=0; i<gen.particles_per_tick; ++i) {
			ecs::entity_id ent = ecs::createEntity(pos, sprite(0xff0000ff), velocity_random(), scale(30.f), shrinking(0.99f), health());
			ecs::addComponent(ent, reflect(ent));
		}
	}
}