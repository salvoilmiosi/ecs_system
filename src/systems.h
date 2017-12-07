#ifndef __SYSTEMS_H__
#define __SYSTEMS_H__

#include <iostream>

#include "components.h"
#include "tuple_util.h"

namespace sys {
	template<typename ... Reqs>
	class system {
	private:
		std::function<void(Reqs &...)> func;
		comp::component_mask i_mask;

	public:
		system(auto func) : func(func) {
			i_mask = 0;
			auto req_list = std::make_tuple(comp::getList<Reqs>() ...);
			for_each_in_tuple(req_list, [this](auto &x){
				i_mask |= x.mask();
			});
		}

		void execute() {
			for (auto &i : comp::mask_list) {
				entity_id ent = i.first;
				entity_id e_mask = i.second;
				if ((e_mask & mask()) == mask()) {
					func(comp::getComponent<Reqs>(ent)...);
				}
			}
		}

		comp::component_mask mask() {
			return i_mask;
		}
	};

	/*************************************
	SYSTEMS DEFINED HERE
	*************************************/

	auto movement = system<comp::position, comp::velocity>([](auto &pos, auto &vel){
		pos.x += vel.x;
		pos.y += vel.y;
	});

	auto print = system<comp::position, comp::sprite>([](auto &pos, auto &spr){
		std::cout << "Position: (" << pos.x << ", " << pos.y << "), Sprite: " << spr.src << std::endl;
	});

	auto draw = system<comp::position, comp::sprite>([](auto &pos, auto &spr){
		// draws entity
	});

	auto all = std::make_tuple(
		movement,
		print,
		draw
	);

	void executeAll() {
		for_each_in_tuple(all, [](auto &x){
			x.execute();
		});
	}
}

#endif // __SYSTEMS_H__