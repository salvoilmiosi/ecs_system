#include "ecs.h"

#include <SDL2/SDL.h>
#include <iostream>

#include <cstdlib>

extern SDL_Renderer *renderer;

namespace ecs {
	void print(entity &me, printable &p, position &pos) {
		std::cout << "Entity " << p.name << "(" << me.id << "), Position (" << pos.x << ", " << pos.y << ")" << std::endl;
	}

	void draw(entity&, sprite &spr, position &pos, scale &s) {
		SDL_Rect rect {(int)(pos.x - s.value * 0.5f), (int)(pos.y - s.value * 0.5f), (int)s.value, (int)s.value};
		Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
		Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
		Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
		Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
		SDL_RenderFillRect(renderer, &rect);
	}

	void tick(entity& me, health &hp) {
		--hp.value;
		if (hp.value <= 0) {
			//std::cout << "Entity " << id << " is dead" << std::endl;
			removeEntity(me);
		}
	}

	position position_random(position orig) {
		orig.x += ((float) rand() / RAND_MAX - 0.5f) * 100.f;
		orig.y += ((float) rand() / RAND_MAX - 0.5f) * 100.f;
		return orig;
	}

	sprite sprite_random() {
		sprite ret;
		Uint8 r = rand() % 0xff;
		Uint8 g = rand() % 0xff;
		Uint8 b = rand() % 0xff;
		Uint8 a = rand() % 0xff;
		ret.color = (r << (8 * 3)) | (g << (8 * 2)) | (b << (8 * 1)) | (a << (8 * 0));
		return ret;
	}

	velocity velocity_random() {
		velocity ret;
		ret.x = ((float) rand() / RAND_MAX - 0.5f) * 8.f;
		ret.y = ((float) rand() / RAND_MAX - 0.5f) * 8.f;
		return ret;
	}

	acceleration acceleration_random() {
		acceleration ret;
		ret.x = ((float) rand() / RAND_MAX - 0.5f) * 0.2f;
		ret.y = ((float) rand() / RAND_MAX - 0.5f) * 0.2f;
		return ret;
	}

	void generate(entity&, position &pos, generator &gen) {
		for (int i=0; i<gen.particles_per_tick; ++i) {
			createEntity(position_random(pos), sprite_random(), velocity_random(), scale(rand() % 15 + 25.f), shrinking(0.983f), health(rand() % 100 + 50), acceleration_random());
		}
	}
}