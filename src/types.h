#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include <tuple>
#include <map>

template<class F, class...Ts, std::size_t...Is>
inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
	using expander = int[];
	(void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func){
	for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

namespace ecs {
	typedef unsigned int entity_id;
	
	typedef unsigned long int component_mask;

	extern std::map<entity_id, component_mask> mask_list;
	
	extern entity_id maxEntityId;

	const inline auto maskListCopy() {
		return mask_list; // TODO create update function
	}
}

#endif // __ENTITIES_H__