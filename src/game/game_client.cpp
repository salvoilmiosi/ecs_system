#include "game_client.h"

namespace game {

void game_client::start() {

}

void game_client::tick() {
	wld.executeSystem<position, velocity>([&](ecs::entity_id id, position &pos, velocity &vel) {
		pos.value.x += vel.value.x;
		pos.value.y += vel.value.y;
	});
	wld.executeSystem<velocity, acceleration>([&](ecs::entity_id id, velocity &vel, acceleration &acc) {
		vel.value.x += acc.value.x;
		vel.value.y += acc.value.y;
	});
	wld.executeSystem<scale, shrinking>([&](ecs::entity_id id, scale &sca, shrinking &shr) {
		sca.value *= shr.value;
	});
	wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
		--hp.value;
		if (hp.value <= 0) {
			wld.removeEntity(me);
		}
	});

	wld.executeSystem<position, generator>([&](ecs::entity_id id, position &pos, generator &gen) {
		generateParticles(id, pos, gen);
	});

	wld.updateEntities();
}

void game_client::render(SDL_Renderer *renderer) {
	wld.executeSystem<sprite, position, scale>([&](ecs::entity_id id, sprite &spr, position &pos, scale &sca) {
		renderEntity(renderer, spr, pos, sca);
	});
}

void game_client::generateParticles(ecs::entity_id, position &pos, generator &gen) {
	for (int i=0; i<5; ++i) {
		Uint8 r = rand() % 0xff;
		Uint8 g = rand() % 0xff;
		Uint8 b = rand() % 0xff;
		Uint8 a = rand() % 0xff;
		sprite sprite_random((r << (8 * 3)) | (g << (8 * 2)) | (b << (8 * 1)) | (a << (8 * 0)));
		
		position position_random(
			pos.value.x + ((float) rand() / RAND_MAX - 0.5f) * 100.f,
			pos.value.y + ((float) rand() / RAND_MAX - 0.5f) * 100.f);

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

void game_client::renderEntity(SDL_Renderer *renderer, sprite &spr, position &pos, scale &s) {
	SDL_Rect rect {(int)(pos.value.x - s.value * 0.5f), (int)(pos.value.y - s.value * 0.5f), (int)s.value, (int)s.value};
	Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
	Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_RenderFillRect(renderer, &rect);
}

}