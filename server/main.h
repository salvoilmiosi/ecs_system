#ifndef __MAIN_H__
#define __MAIN_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "ecs_net.h"
#include "components.h"

namespace server {

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;
static const int FPS = 60;

static const int MAX_ENTITIES = ecs::MAX_ENTITIES_DEFAULT;

extern ecs::world_out<MyComponents, MAX_ENTITIES> wld;

extern SDL_Window *window;
extern SDL_Renderer *renderer;

void handleMouse(IPaddress addr, SDL_MouseButtonEvent mouse);

}

#endif // __MAIN_H__