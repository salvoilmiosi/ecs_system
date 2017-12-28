#ifndef __USERINPUT_H__
#define __USERINPUT_H__

#include <SDL2/SDL.h>

#include "ecs/world_io.h"

#include "components_serial.h"
#include "systems.h"

namespace game::userinput {

enum command_type {
	CMD_NONE,
	CMD_UP,
	CMD_DOWN,
	CMD_MOVE
};

struct command {
	command_type cmd;
	position pos;
};

class handler {
public:
	void handleCommand(ecs::world_io<MyComponents> &wld, command cmd);

private:
	ecs::entity_id ent = 0;
};

command handleEvent(const SDL_Event &event);

}

#endif // __USERINPUT_H__
