#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>
#include <functional>

#include "entities.h"

namespace ecs {
	template<class F, class...Ts, std::size_t...Is>
	inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
		using expander = int[];
		(void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
	}

	template<class F, class...Ts>
	inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func){
		for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
	}

	template<typename ... Ts> inline component_mask getMask() {
		return 0;
		// TODO
	}

	template<typename ... Ts>
	class system {
	private:
		std::function<void(entity&, Ts& ...)> func;
		const component_mask mask;

	public:
		system(auto func) : func(func), mask(getMask<Ts...>()) {}

		template<typename Components, typename Entities>
		void execute(Components comp, Entities ents) {
			for (entity &ent : ents) {
				if ((ent.mask & mask) == mask) {
					func(ent, comp.template getComponent<Ts>(ent) ...);
				}
			}
		}
	};
}

#endif // __TYPES_H__