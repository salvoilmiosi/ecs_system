#ifndef __MAIN_H__
#define __MAIN_H__

#include <SDL2/SDL.h>

#include "ecs_net.h"
#include "components.h"

namespace client {

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;
static const int FPS = 60;

static const int MAX_ENTITIES = ecs::MAX_ENTITIES_DEFAULT;

extern ecs::world_in<MyComponents, MAX_ENTITIES> wld;

extern SDL_Window *window;
extern SDL_Renderer *renderer;

enum packet_type {
	PACKET_NONE,

	PACKET_EDITLOG,
	PACKET_SERVERMSG,

	PACKET_USER_COMMAND,
	PACKET_USER_INPUT
};

}

#endif // __MAIN_H__