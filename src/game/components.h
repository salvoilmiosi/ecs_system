#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <string>

#include <SDL2/SDL.h>

#include "ecs/world.h"
#include "ecs/packet_data.h"

#include "vector.h"

template<typename T>
struct component {
	T value;

	component(T value) : value(value) {}

	void read(packet_reader&);
	void write(packet_writer&) const;
};

struct color : component<int> {
	color(int value = 0) : component<int>(value) {}
};

struct square : ecs::tag { };

struct circle : ecs::tag { };

struct position : component<vector2d> {
	position(double x = 0.f, double y = 0.f) : component<vector2d>(vector2d(x, y)) {};
};

struct rotation : component<float> {
	rotation(float value = 0.f) : component<float>(value) {}
};

struct rotation_accel : component<float> {
	rotation_accel(float value = 0.f) : component<float>(value) {}
};

struct velocity : component<vector2d> {
	velocity(double x = 0.f, double y = 0.f) : component<vector2d>(vector2d(x, y)) {};
};

struct acceleration : component<vector2d> {
	acceleration(double x = 0.f, double y = 0.f) : component<vector2d>(vector2d(x, y)) {};
};

struct scale : component<float> {
	scale(float value = 1.f) : component<float>(value) {}
};

struct shrinking : component<float> {
	shrinking(float value = 1.f) : component<float>(value) {}
};

struct health : component<int> {
	health(int value = 100) : component<int>(value) {}
};

struct generator : ecs::tag { };

struct dying : ecs::tag { };

struct collision : ecs::tag {};

struct screen_bounce : ecs::tag { };

using MyComponents = ecs::component_list<
	color, square, circle, position, rotation, rotation_accel, velocity, acceleration, scale, shrinking, health, generator, dying, collision, screen_bounce>;

#endif