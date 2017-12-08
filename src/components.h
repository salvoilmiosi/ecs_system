#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include "ecs.h"

struct printable {
	const char *name;

	printable() {}
	printable(const char *name) : name(name) {}
};

struct sprite {
	const char *src;
	int color;

	sprite() {}
	sprite(const char *src) : src(src) {}
	sprite(int color) : color(color) {}
};

struct position {
	float x, y;

	position() {
		x = 0.f;
		y = 0.f;
	}
	position(float x, float y) : x(x), y(y) {}
};

struct velocity {
	float x, y;

	velocity() {
		x = 0.0f;
		y = 0.0f;
	}
	velocity(float x, float y) : x(x), y(y) {}
};

struct acceleration {
	float x, y;

	acceleration() {
		x = 0.f;
		y = 0.f;
	}
	acceleration(float x, float y) : x(x), y(y) {}
};

struct scale {
	float value;

	scale() {
		value = 1.f;
	}
	scale(float value) : value(value) {}
};

struct shrinking {
	float value;

	shrinking() {
		value = 1.f;
	}
	shrinking(float value) : value(value) {}
};

struct health {
	int value;

	health() {
		value = 100;
	}
	health(int value) : value(value) {}
};

struct generator {
	int particles_per_tick;

	generator() {
		particles_per_tick = 7;
	}
	generator(int ptt) : particles_per_tick(ptt) {}
};

using MyComponents = ecs::component_list<
	printable, sprite, position, velocity, acceleration, scale, shrinking, health, generator>;

#endif