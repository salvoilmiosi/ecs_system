#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <string>

#include <SDL2/SDL.h>

#include "ecs/world.h"
#include "ecs/packet_data.h"

#include "vector.h"

struct int_component {
	int value;

	int_component(int value) : value(value) {}

	void read(packet_reader&);
	void write(packet_writer&) const;
};

struct float_component {
	float value;

	float_component(float value) : value(value) {}

	void read(packet_reader&);
	void write(packet_writer&) const;
};

struct vec_component {
	vector2d value;

	vec_component(float x = 0.f, float y = 0.f) : value{x, y} {}

	void read(packet_reader&);
	void write(packet_writer&) const;
};

struct color : int_component {
	color(int value = 0) : int_component(value) {}
};

struct square : ecs::tag { };

struct circle : ecs::tag { };

struct position : vec_component {
	position(float x = 0.f, float y = 0.f) : vec_component(x, y) {};
};

struct rotation : float_component {
	rotation(float value = 0.f) : float_component(value) {}
};

struct rotation_accel : float_component {
	rotation_accel(float value = 0.f) : float_component(value) {}
};

struct velocity : vec_component {
	velocity(float x = 0.f, float y = 0.f) : vec_component(x, y) {};
};

struct acceleration : vec_component {
	acceleration(float x = 0.f, float y = 0.f) : vec_component(x, y) {};
};

struct scale : float_component {
	scale(float value = 1.f) : float_component(value) {}
};

struct shrinking : float_component {
	shrinking(float value = 1.f) : float_component(value) {}
};

struct health : int_component {
	health(int value = 100) : int_component(value) {}
};

struct generator : ecs::tag { };

struct dying : ecs::tag { };

struct collision : ecs::tag {};

struct screen_bounce : ecs::tag { };

using MyComponents = ecs::component_list<
	color, square, circle, position, rotation, rotation_accel, velocity, acceleration, scale, shrinking, health, generator, dying, collision, screen_bounce>;

#endif