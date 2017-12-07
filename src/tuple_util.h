#ifndef __TUPLE_UTIL_H__
#define __TUPLE_UTIL_H__

template<class F, class...Ts, std::size_t...Is>
inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func, std::index_sequence<Is...>){
    using expander = int[];
    (void)expander { 0, ((void)func(std::get<Is>(tuple)), 0)... };
}

template<class F, class...Ts>
inline void for_each_in_tuple(std::tuple<Ts...> & tuple, F func){
    for_each_in_tuple(tuple, func, std::make_index_sequence<sizeof...(Ts)>());
}

#endif //__TUPLE_UTIL_H__