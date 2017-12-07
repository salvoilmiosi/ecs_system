#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include "types.h"

namespace components {
	extern ecs::component_mask maxMask;

	template<typename T> class component_list : public std::map<ecs::entity_id, T> {
	private:
		ecs::component_mask i_mask;

	public:
		component_list() : std::map<ecs::entity_id, T>() {
			i_mask = maxMask;
			maxMask <<= 1;
		}

		ecs::component_mask mask() {
			return i_mask;
		}
	};

	/************************************************
	COMPONENTS DEFINED HERE
	************************************************/

	struct reflect {
		ecs::entity_id id;

		reflect() {}
		reflect(ecs::entity_id id) : id(id) {}
	};

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
			particles_per_tick = 1;
		}
		generator(int ptt) : particles_per_tick(ptt) {}
	};

	/************************************************
	************************************************/
	template<typename ... Ts> static constexpr auto createComponentLists() {
		return std::make_tuple(component_list<Ts>()...);
	}

	inline auto &all() {
		static auto obj = createComponentLists<
			reflect,
			printable,
			sprite,
			position,
			velocity,
			acceleration,
			scale,
			shrinking,
			health,
			generator
		>();
		return obj;
	}

	template<typename T> inline component_list<T> &getList() {
		return std::get<component_list<T>>(all());
	}
	
	template<typename T> inline T &getComponent(ecs::entity_id ent) {
		return components::getList<T>().at(ent);
	}
}

#endif // __COMPONENTS_H__