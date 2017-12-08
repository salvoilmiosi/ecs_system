#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>
#include <array>
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

	template<typename T>
	constexpr T or_all(const T &obj) {
		return obj;
	}

	template<typename T, typename ... Ts>
	constexpr T or_all(const T &first, const Ts& ... then) {
		return first | or_all(then ...);
	}

	template<typename T, size_t Size = MAX_ENTITIES>
	class component_data : public std::array<T, Size> {
	public:
		const component_mask mask;

		component_data(size_t &n) : mask(1 << n++) {}
	};

	template<typename ... Components>
	class component_list {
	private:
		constexpr auto createComponentLists() {
			size_t counter = 0;
			return std::make_tuple(component_data<Components>(counter)...);
		}

		std::tuple<component_data<Components>...> data {createComponentLists()};
	public:
		template<typename T>
		constexpr component_data<T> &getList() {
			return std::get<component_data<T>>(data);
		}
		
		template<typename ... Ts>
		constexpr component_mask getMask() {
			return or_all(getList<Ts>().mask ...);
		}

		template<typename T>
		constexpr T &getComponent(entity ent) {
			return getList<T>()[ent.id];
		}
	};

	template<typename ... Ts>
	class system {
	private:
		std::function<void(entity&, Ts& ...)> func;

	public:
		system(auto func) : func(func) {}

		template<typename Components, typename Entities>
		void execute(Components &comp, Entities &ents) {
			static component_mask mask = comp.template getMask<Ts ...>();
			for (entity &ent : ents) {
				if ((ent.mask & mask) == mask) {
					func(ent, comp.template getComponent<Ts>(ent) ...);
				}
			}
		}
	};
}

#endif // __TYPES_H__