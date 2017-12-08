#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>
#include <array>

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

	typedef unsigned int entity_id;

	typedef unsigned long int component_mask;
}

#endif // __TYPES_H__