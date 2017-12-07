#ifndef __SYSTEMS_H__
#define __SYSTEMS_H__

#include <functional>
#include <iostream>

#include "components.h"

namespace systems {
	template<typename ... Reqs>
	class system {
	private:
		std::function<void(Reqs &...)> func;
		ecs::component_mask i_mask;

	public:
		system(auto func) : func(func) {
			i_mask = 0;
			auto req_list = std::make_tuple(components::getList<Reqs>() ...);
			for_each_in_tuple(req_list, [this](auto &x){
				i_mask |= x.mask();
			});
		}

		void execute() {
			auto mlc = components::mask_list();
			for (ecs::entity_id ent = 0; ent < MAX_ENTITIES; ++ent) {
				ecs::entity_id e_mask = mlc[ent];
				if ((e_mask & mask()) == mask()) {
					try {
						func(components::getComponent<Reqs>(ent)...);
					} catch (std::out_of_range) {
						std::cerr << "Entity " << ent << " not found" << std::endl;
					}
				}
			}
		}

		ecs::component_mask mask() {
			return i_mask;
		}
	};

	using namespace components;

	void print(reflect&, printable&, position&);

	void draw(sprite&, position&, scale&);

	void tick(reflect&, health&);

	void generate(position&, generator&);

	inline auto &all() {
		static auto obj = std::make_tuple(
			system<reflect, printable, position>(print),

			system<sprite, position, scale>(draw),

			system<position, velocity>([](auto &pos, auto &vel) { // move
				pos.x += vel.x;
				pos.y += vel.y;
			}),

			system<velocity, acceleration>([](auto &vel, auto &acc) { // accelerate
				vel.x += acc.x;
				vel.y += acc.y;
			}),

			system<scale, shrinking>([](auto &sca, auto &shr) { // shrinking
				sca.value *= shr.value;
			}),

			system<reflect, health>(tick),

			system<position, generator>(generate)
		);
		return obj;
	}
}

#endif // __SYSTEMS_H__