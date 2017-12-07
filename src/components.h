#ifndef __COMPONENTS_H__
#define __COMPONENTS_H__

#include <tuple>
#include <map>

typedef int entity_t;

namespace comp {
	typedef long int component_mask;

	static component_mask maxMask = 1;

	template<typename T> class component_list : public std::map<entity_t, T> {
	private:
		const component_mask i_mask;

	public:
		component_list() : std::map<entity_t, T>(), i_mask(maxMask) {
			maxMask <<= 1;
		}

		component_mask mask() {
			return i_mask;
		}
	};

	std::map<entity_t, component_mask> mask_list;

	/************************************************
	COMPONENTS DEFINED HERE
	************************************************/

	struct position {
		float x, y;

		position() {}
		position(float x, float y) : x(x), y(y) {}
	};

	struct velocity {
		float x, y;

		velocity() {}
		velocity(float x, float y) : x(x), y(y) {}
	};

	struct sprite {
		const char *src;

		sprite() {}
		sprite(const char *src) : src(src) {}
	};
	
	auto all = std::make_tuple(
		component_list<position>(),
		component_list<velocity>(),
		component_list<sprite>()
	);

	template<typename T> inline component_list<T> &getList() {
		return std::get<component_list<T>>(all);
	}

	template<typename T> inline T &getComponent(entity_t ent) {
		return getList<T>().at(ent);
	}
}

#endif // __COMPONENTS_H__