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

	inline command handleEvent(const SDL_Event &event) {
		command cmd;
		switch(event.type) {
		case SDL_MOUSEBUTTONDOWN:
			cmd.cmd = CMD_DOWN;
			cmd.pos.x = event.button.x;
			cmd.pos.y = event.button.y;
			break;
		case SDL_MOUSEBUTTONUP:
			cmd.cmd = CMD_UP;
			cmd.pos.x = event.button.x;
			cmd.pos.y = event.button.y;
			break;
		case SDL_MOUSEMOTION:
			cmd.cmd = CMD_MOVE;
			cmd.pos.x = event.motion.x;
			cmd.pos.y = event.motion.y;
			break;
		default:
			cmd.cmd = CMD_NONE;
			break;
		}
		return cmd;
	}
};

#endif // __USERINPUT_H__
