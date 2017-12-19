#ifndef __USERINPUT_H__
#define __USERINPUT_H__

#include <SDL2/SDL.h>

#include "ecs.h"

class userinput {
public:
	void handleMouseButton(const SDL_MouseButtonEvent &event);
	void handleMouseMotion(const SDL_MouseMotionEvent &event);

private:
	ecs::entity_id ent = 0;
};

#endif // __USERINPUT_H__
