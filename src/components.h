#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include "ecs.h"

struct printable {
	const char *name;

	printable() {}
	printable(const char *name) : name(name) {}
};

struct sprite {
	const char *src = nullptr;
	int color = 0;

	sprite() {}
	sprite(const char *src) : src(src) {}
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
	int particles_per_tick;
	float scaled;

	generator(int ptt = 7, float scaled = 1.f) : particles_per_tick(ptt), scaled(scaled) {}
};

using MyComponents = ecs::component_list<
	printable, sprite, position, velocity, acceleration, scale, shrinking, health, generator>;

#endif