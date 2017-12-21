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

command handleEvent(const SDL_Event &event);

}

#endif // __USERINPUT_H__
