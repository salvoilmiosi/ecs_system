#ifndef __GAME_CLIENT_H__
#define __GAME_CLIENT_H__

#include <SDL2/SDL.h>

#include "ecs/world_io.h"

#include "components.h"

namespace game {

class game_client {
public:
	void start();
	void tick();

	void render(SDL_Renderer *renderer);

	void applyEdits(auto &edits) {
		wld.applyEdits(edits);
	}

private:
	ecs::world_in<MyComponents> wld;

	void generateParticles(ecs::entity_id, position &pos);

	void renderEntity(SDL_Renderer *renderer, sprite &spr, position &pos, scale &s);
};

}

#endif // __GAME_CLIENT_H__