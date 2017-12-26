#ifndef __USERINPUT_H__
#define __USERINPUT_H__

#include <SDL2/SDL.h>

#include "components.h"

namespace userinput {

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
	void handleCommand(auto &wld, command cmd);

private:
	ecs::entity_id ent = 0;
};

void handler::handleCommand(auto &wld, command cmd) {
	switch (cmd.cmd) {
	case CMD_DOWN:
		ent = wld.createEntity(cmd.pos, generator(5));
		break;
	case CMD_UP:
		wld.addComponent(ent, health(5));
		ent = 0;
		break;
	case CMD_MOVE:
		if (ent) {
			wld.addComponent(ent, cmd.pos);
		}
		break;
	case CMD_NONE:
	default:
		break;
	}
}

command handleEvent(const SDL_Event &event);

}

#endif // __USERINPUT_H__
