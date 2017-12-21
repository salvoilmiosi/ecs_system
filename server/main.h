#ifndef __MAIN_H__
#define __MAIN_H__

#include "ecs_net.h"
#include "components.h"

namespace server {

static const int TICKRATE = 60;

static const int MAX_ENTITIES = ecs::MAX_ENTITIES_DEFAULT;

extern ecs::world_out<MyComponents, MAX_ENTITIES> wld;

enum packet_type {
	PACKET_NONE,

	PACKET_EDITLOG,
	PACKET_SERVERMSG,

	PACKET_USER_COMMAND,
	PACKET_USER_INPUT
};

}

#endif // __MAIN_H__