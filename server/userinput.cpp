#include "userinput.h"
#include "main.h"

void userinput::handleMouseButton(const SDL_MouseButtonEvent &event) {
	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
		ent = server::wld.createEntity(position(event.x, event.y), generator(5));
		break;
	case SDL_MOUSEBUTTONUP:
		server::wld.addComponent(ent, health(5));
		ent = 0;
		break;
	default:
		break;
	}
}

void userinput::handleMouseMotion(const SDL_MouseMotionEvent &event) {
	if (ent) {
		server::wld.addComponent(ent, position(event.x, event.y));
	}
}
