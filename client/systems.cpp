#include "systems.h"

#include <cstdlib>

#include "main.h"

void print_func(ecs::entity_id me, printable &p, position &pos) {
	std::cout << "Entity " << p.name << "(" << me << "), Position (" << pos.x << ", " << pos.y << ")" << std::endl;
};

void move_func(ecs::entity_id, position &pos, velocity &vel) {
	pos.x += vel.x;
	pos.y += vel.y;
};

void accelerate_func(ecs::entity_id, velocity &vel, acceleration &acc) {
	vel.x += acc.x;
	vel.y += acc.y;
};

void shrink_func(ecs::entity_id, scale &sca, shrinking &shr) {
	sca.value *= shr.value;
};

void draw_func(ecs::entity_id, sprite &spr, position &pos, scale &s) {
	SDL_Rect rect {(int)(pos.x - s.value * 0.5f), (int)(pos.y - s.value * 0.5f), (int)s.value, (int)s.value};
	Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
	Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
	SDL_SetRenderDrawColor(client::renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(client::renderer, SDL_BLENDMODE_ADD);
	SDL_RenderFillRect(client::renderer, &rect);
}

void health_tick_func(ecs::entity_id me, health &hp) {
	--hp.value;
	if (hp.value <= 0) {
		client::wld.removeEntity(me);
	}
}