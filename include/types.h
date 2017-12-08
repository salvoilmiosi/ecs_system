#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>
#include <array>
#include <bitset>

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
	
	static const size_t MAX_ENTITIES = 4096;
	static const size_t COMPONENT_NUM = 32;

	typedef size_t entity_id;

	typedef std::bitset<COMPONENT_NUM> component_mask;
	
	struct entity {
		entity_id id;
		component_mask mask;
		bool alive;

		entity() {
			id = 0;
			mask = 0;
			alive = false;
		}
	};
}

#endif // __TYPES_H__