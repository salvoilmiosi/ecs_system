#ifndef __TYPES_H__
#define __TYPES_H__

#include <tuple>
#include <array>
#include <functional>

#include "entities.h"

namespace ecs {
	static const size_t MAX_ENTITIES_DEFAULT = 100;

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

	template<typename Component, size_t CompSize, size_t Size>
	class component_entry : public std::array<Component, Size> {
	public:
		const component_mask<CompSize> mask;

		component_entry(size_t &n) : mask(1 << n++) {}
	};

	template<typename ... Components>
	class component_list {
	public:
		static constexpr size_t size() {
			return sizeof...(Components);
		}

		template<size_t MaxEntities>
		using ComponentData = std::tuple<component_entry<Components, size(), MaxEntities>...>;

		template<size_t MaxEntities>
		constexpr auto createComponentData() {
			size_t counter = 0;
			return std::make_tuple(component_entry<Components, size(), MaxEntities>(counter)...);
		}
	};

	template<typename ... Ts>
	class system {
	private:
		std::function<void(entity_id, Ts& ...)> func;

	public:
		system(auto func) : func(func) {}

		void execute(auto &world, auto &ents) {
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