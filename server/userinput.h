#ifndef __USERINPUT_H__
#define __USERINPUT_H__

#include <SDL2/SDL.h>

#include "ecs.h"
#include "components.h"

enum command_type {
	CMD_NONE,
	CMD_UP,
	CMD_DOWN,
	CMD_MOVE
};

struct input_command {
	command_type cmd;
	position pos;
};

class userinput {
public:
	void handleCommand(input_command cmd);

private:
	ecs::entity_id ent = 0;
};

#endif // __USERINPUT_H__
