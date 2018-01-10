#include "userinput.h"

namespace game::userinput {

void handler::handleCommand(ecs::world_io<MyComponents> &wld, command cmd) {
	switch (cmd.cmd) {
	case CMD_DOWN:
		ent = wld.createEntity(cmd.pos, generator());
		break;
	case CMD_UP:
		wld.addComponents(ent, health(5), dying());
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

command handleEvent(const SDL_Event &event) {
	command cmd;
	cmd.cmd = CMD_NONE;

	switch(event.type) {
	case SDL_MOUSEBUTTONDOWN:
		if (event.button.button == SDL_BUTTON_LEFT) {
			cmd.cmd = CMD_DOWN;
			cmd.pos.value.x = event.button.x;
			cmd.pos.value.y = event.button.y;
		}
		break;
	case SDL_MOUSEBUTTONUP:
		if (event.button.button == SDL_BUTTON_LEFT) {
			cmd.cmd = CMD_UP;
			cmd.pos.value.x = event.button.x;
			cmd.pos.value.y = event.button.y;
		}
		break;
	case SDL_MOUSEMOTION:
		cmd.cmd = CMD_MOVE;
		cmd.pos.value.x = event.motion.x;
		cmd.pos.value.y = event.motion.y;
		break;
	default:
		break;
	}
	return cmd;
}

}