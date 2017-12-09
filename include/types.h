#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>
#include <array>
#include <functional>

#include "entities.h"
#include "mpl.h"

namespace ecs {
	static const size_t MAX_ENTITIES_DEFAULT = 4096;

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

	template<typename Component, size_t Size>
	using component_entry = std::array<Component, Size>;

	template<typename ... Components>
	using component_list = mpl::TypeList<Components...>;

	template<typename ... Ts>
	class system {
	private:
		std::function<void(entity_id, Ts& ...)> func;

	public:
		system(auto _func) {
			static_assert(std::is_assignable<decltype(func), decltype(_func)>{});
			func = _func;
		}

		void execute(auto &world, auto &ents) {
			static_assert(world.template areAllComponents<Ts...>());
			static auto mask = world.template generateMask<Ts ...>();
			for (auto &ent : ents) {
				if ((ent.mask & mask) == mask) {
					func(ent.id, world.template getComponent<Ts>(ent.id) ...);
				}
			}
		}
	};
}

#endif // __TYPES_H__