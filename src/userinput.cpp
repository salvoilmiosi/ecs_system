#include "userinput.h"

#include "systems.h"

namespace userinput {

command handleEvent(const SDL_Event &event) {
	command cmd;
	cmd.cmd = CMD_NONE;

	switch(event.type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT) {
			cmd.cmd = CMD_DOWN;
			cmd.pos.x = event.button.x;
			cmd.pos.y = event.button.y;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == SDL_BUTTON_LEFT) {
			cmd.cmd = CMD_UP;
			cmd.pos.x = event.button.x;
			cmd.pos.y = event.button.y;
		}
		break;
	case SDL_MOUSEMOTION:
		cmd.cmd = CMD_MOVE;
		cmd.pos.x = event.motion.x;
		cmd.pos.y = event.motion.y;
		break;
	default:
		break;
	}
	return cmd;
}

}