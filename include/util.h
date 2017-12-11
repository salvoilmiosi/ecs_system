#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>

#include "mpl.h"

template<class F, class...Ts, std::size_t...Is>
inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
	using expander = int[];
	(void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func){
	for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

template<typename T>
constexpr T or_all(const T &obj) {
	return obj;
}

template<typename T, typename ... Ts>
constexpr T or_all(const T &first, const Ts& ... then) {
	return first | or_all(then ...);
}

#endif // __TYPES_H__