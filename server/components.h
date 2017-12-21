#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <string>

#include "ecs.h"

struct printable {
	std::string name;

	printable() {}
	printable(std::string name) : name(name) {}
};

struct sprite {
	std::string src;
	int color = 0;

	sprite() {}
	sprite(std::string src) : src(src) {}
	sprite(int color) : color(color) {}
};

struct position {
	float x = 0.f;
	float y = 0.f;

	position() {}
	position(float x, float y) : x(x), y(y) {}
};

struct velocity {
	float x = 0.f;
	float y = 0.f;

	velocity() {}
	velocity(float x, float y) : x(x), y(y) {}
};

struct acceleration {
	float x = 0.f;
	float y = 0.f;

	acceleration() {}
	acceleration(float x, float y) : x(x), y(y) {}
};

struct scale {
	float value = 1.f;

	scale() {}
	scale(float value) : value(value) {}
};

struct shrinking {
	float value = 1.f;

	shrinking() {}
	shrinking(float value) : value(value) {}
};

struct health {
	int value = 100;

	health() {}
	health(int value) : value(value) {}
};

struct generator {
	int particles_per_tick = 7;

	generator() {}
	generator(int ptt) : particles_per_tick(ptt) {}
};

using MyComponents = ecs::component_list<
	printable, sprite, position, velocity, acceleration, scale, shrinking, health, generator>;

#endif