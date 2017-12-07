#ifndef __SYSTEMS_H__
#define __SYSTEMS_H__

#include <functional>
#include <iostream>

#include "components.h"

namespace ecs {
	template<typename ... Reqs>
	class system {
	private:
		std::function<void(entity_id, Reqs &...)> func;
		component_mask i_mask;

	public:
		system(auto func) : func(func) {
			i_mask = 0;
			auto req_list = std::make_tuple(getList<Reqs>() ...);
			for_each_in_tuple(req_list, [this](auto &x){
				i_mask |= x.mask();
			});
		}

		void execute() {
			auto &mlc = mask_list();
			for (entity_id ent = 0; ent < MAX_ENTITIES; ++ent) {
				entity_id e_mask = mlc[ent];
				if ((e_mask & mask()) == mask()) {
					func(ent, getComponent<Reqs>(ent)...);
				}
			}
		}

		component_mask mask() {
			return i_mask;
		}
	};

	void print(entity_id, printable&, position&);

	void draw(entity_id, sprite&, position&, scale&);

	void tick(entity_id, health&);

	void generate(entity_id, position&, generator&);

	inline auto &allSystems() {
		static auto obj = std::make_tuple(
			system<printable, position>(print),

			system<sprite, position, scale>(draw),

			system<position, velocity>([](entity_id, auto &pos, auto &vel) { // move
				pos.x += vel.x;
				pos.y += vel.y;
			}),

			system<velocity, acceleration>([](entity_id, auto &vel, auto &acc) { // accelerate
				vel.x += acc.x;
				vel.y += acc.y;
			}),

			system<scale, shrinking>([](entity_id, auto &sca, auto &shr) { // shrinking
				sca.value *= shr.value;
			}),

			system<health>(tick),

			system<position, generator>(generate)
		);
		return obj;
	}
}

#endif // __SYSTEMS_H__