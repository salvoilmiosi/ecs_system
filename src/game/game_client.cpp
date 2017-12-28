#include "game_client.h"

#include "systems.h"

namespace game {

void game_client::tick() {
	wld.executeSystem<position, generator>([&](ecs::entity_id id, position &pos, generator &gen) {
		particle_generator_func(id, pos, gen);
	});
	wld.executeSystem<position, velocity>(move_func);
	wld.executeSystem<velocity, acceleration>(accelerate_func);
	wld.executeSystem<scale, shrinking>(shrink_func);
	wld.executeSystem<health>(health_tick_func);
	wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
		if (hp.value <= 0) {
			wld.removeEntity(me);
		}
	});

	wld.updateEntities();
}

void game_client::render() {
	wld.executeSystem<sprite, position, scale>([&](ecs::entity_id id, sprite &spr, position &pos, scale &sca) {
		draw_func(id, spr, pos, sca);
	});

	SDL_RenderPresent(renderer);
}

void game_client::particle_generator_func(ecs::entity_id, position &pos, generator &gen) {
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
			// will create on average 2000 entities
		} catch (std::out_of_range) {
			// out of memory
			return;
		}
	}
}

void game_client::draw_func(ecs::entity_id, sprite &spr, position &pos, scale &s) {
	SDL_Rect rect {(int)(pos.x - s.value * 0.5f), (int)(pos.y - s.value * 0.5f), (int)s.value, (int)s.value};
	Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
	Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_RenderFillRect(renderer, &rect);
}

}