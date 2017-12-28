#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL.h>

#include "ecs/world_io.h"

#include "components_serial.h"

namespace game {

class game_client {
public:
	game_client(SDL_Renderer* const& renderer) : renderer(renderer) {}

	void tick();
	void render();

	void applyEdits(auto &edits) {
		wld.applyEdits(edits);
	}

private:
	ecs::world_in<MyComponents> wld;

	SDL_Renderer* const& renderer;

	void particle_generator_func(ecs::entity_id, position &pos, generator &gen);

	void draw_func(ecs::entity_id, sprite &spr, position &pos, scale &s);
};

}

#endif // __GAME_CLIENT_H__