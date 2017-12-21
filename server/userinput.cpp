#include "userinput.h"
#include "main.h"

void userinput::handleCommand(input_command cmd) {
	switch (cmd.cmd) {
	case CMD_DOWN:
		ent = server::wld.createEntity(cmd.pos, generator(5));
		break;
	case CMD_UP:
		server::wld.addComponent(ent, health(5));
		ent = 0;
		break;
	case CMD_MOVE:
		if (ent) {
			server::wld.addComponent(ent, cmd.pos);
		}
		break;
	case CMD_NONE:
	default:
		break;
	}
}