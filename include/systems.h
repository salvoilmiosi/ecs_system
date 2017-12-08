#ifndef __SYSTEMS_H__
#define __SYSTEMS_H__

#include <functional>

#include "components.h"
#include "entity_list.h"

namespace ecs {
	template<typename ... Reqs>
	class system {
	private:
		std::function<void(entity&, Reqs &...)> func;
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
			for (entity &ent : allEntities()) {
				if ((ent.mask & mask()) == mask()) {
					func(ent, getComponent<Reqs>(ent)...);
				}
			}
		}

		component_mask mask() {
			return i_mask;
		}
	};

	/************************************************
	SYSTEMS DEFINED HERE
	************************************************/

	void print(entity&, printable&, position&);

	void draw(entity&, sprite&, position&, scale&);

	void tick(entity&, health&);

	void generate(entity&, position&, generator&);

	inline auto &allSystems() {
		static auto obj = std::make_tuple(
			system<printable, position>(print),

			system<sprite, position, scale>(draw),

			system<position, velocity>([](entity&, auto &pos, auto &vel) { // move
				pos.x += vel.x;
				pos.y += vel.y;
			}),

			system<velocity, acceleration>([](entity&, auto &vel, auto &acc) { // accelerate
				vel.x += acc.x;
				vel.y += acc.y;
			}),

			system<scale, shrinking>([](entity&, auto &sca, auto &shr) { // shrinking
				sca.value *= shr.value;
			}),

			system<health>(tick),

			system<position, generator>(generate)
		);
		return obj;
	}
	
	/************************************************
	************************************************/
	
	inline void executeAllSystems() {
		for_each_in_tuple(allSystems(), [](auto &x){
			x.execute();
		});
	}
}

#endif // __SYSTEMS_H__