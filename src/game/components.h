#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <string>

#include "ecs/world.h"
#include "ecs/packet_data.h"

struct vector2d {
	float x;
	float y;
};

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

struct sprite {
	std::string src;
	int color = 0;

	sprite() {}
	sprite(std::string src) : src(src) {}
	sprite(int color) : color(color) {}

	void read(packet_reader&);
	void write(packet_writer&) const;
};

struct position : vec_component {
	position(float x = 0.f, float y = 0.f) : vec_component(x, y) {};
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

using MyComponents = ecs::component_list<
	sprite, position, velocity, acceleration, scale, shrinking, health, generator>;

#endif