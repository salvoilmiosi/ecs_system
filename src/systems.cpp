#include "systems.h"

#include <SDL2/SDL.h>
#include <iostream>

#include <cstdlib>

extern SDL_Renderer *renderer;

extern ecs::world<MyComponents> wld;

void print_func(ecs::entity &me, printable &p, position &pos) {
	std::cout << "Entity " << p.name << "(" << me.id << "), Position (" << pos.x << ", " << pos.y << ")" << std::endl;
};

void move_func(ecs::entity&, position &pos, velocity &vel) {
	pos.x += vel.x;
	pos.y += vel.y;
};

void accelerate_func(ecs::entity&, velocity &vel, acceleration &acc) {
	vel.x += acc.x;
	vel.y += acc.y;
};

void shrink_func(ecs::entity&, scale &sca, shrinking &shr) {
	sca.value *= shr.value;
};

void draw_func(ecs::entity&, sprite &spr, position &pos, scale &s) {
	SDL_Rect rect {(int)(pos.x - s.value * 0.5f), (int)(pos.y - s.value * 0.5f), (int)s.value, (int)s.value};
	Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
	Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_RenderFillRect(renderer, &rect);
}

void health_tick_func(ecs::entity& me, health &hp) {
	--hp.value;
	if (hp.value <= 0) {
		//std::cout << "Entity " << id << " is dead" << std::endl;
		wld.removeEntity(me);
	}
}

void particle_generator_func(ecs::entity&, position &pos, generator &gen) {
	for (int i=0; i<gen.particles_per_tick; ++i) {
		Uint8 r = rand() % 0xff;
		Uint8 g = rand() % 0xff;
		Uint8 b = rand() % 0xff;
		Uint8 a = rand() % 0xff;
		sprite sprite_random((r << (8 * 3)) | (g << (8 * 2)) | (b << (8 * 1)) | (a << (8 * 0)));
		
		position position_random(
			pos.x + ((float) rand() / RAND_MAX - 0.5f) * 100.f,
			pos.y + ((float) rand() / RAND_MAX - 0.5f) * 100.f);

		velocity velocity_random(
			((float) rand() / RAND_MAX - 0.5f) * 8.f,
			((float) rand() / RAND_MAX - 0.5f) * 8.f);

		acceleration acceleration_random(
			((float) rand() / RAND_MAX - 0.5f) * 0.2f,
			((float) rand() / RAND_MAX - 0.5f) * 0.2f);

		try {
			wld.createEntity(position_random, sprite_random, velocity_random, acceleration_random,
				scale(rand() % 15 + 25.f), shrinking(0.983f), health(rand() % 100 + 50));
		} catch (std::out_of_range) {
			// out of memory
			return;
		}
	}
}