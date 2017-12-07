#include <SDL2/SDL.h>

#include "ecs.h"

using namespace components;

static const int WIDTH = 400;
static const int HEIGHT = 400;

int main(int argc, char **argv) {
	auto id = ecs::createEntity(printable("Generator"), position(WIDTH / 2.0, HEIGHT / 2.0), generator());
	ecs::addComponent(id, reflect(id));
	for(int i=0; i<100; i++) {
		ecs::executeAllSystems();
	}
	return 0;
}