#include <iostream>

#include "ecs.h"

int main(int argc, char const *argv[]) {
	entity_id ent = createEntity();//comp::position(0, 0), comp::sprite("sprite.png"));

	sys::executeAll();

	addComponent(ent, comp::position(0,0));
	addComponent(ent, comp::sprite("sprite.png"));

	sys::executeAll();

	addComponent(ent, comp::velocity(10, 10));

	sys::executeAll();

	return 0;
}