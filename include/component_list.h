#ifndef __COMPONENT_LIST_H__
#define __COMPONENT_LIST_H__

#include "types.h"

namespace ecs {
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

		std::tuple<component_data<Components>...> data = createComponentLists();
	public:
		template<typename ... Ts>
		const component_mask getMask() {
			component_mask n = 0;
			auto tup = std::make_tuple(Ts()...); // THERE HAS TO BE A BETTER WAY
			for_each_in_tuple([n](auto &x){
				n |= std::get<component_data<x>>(data).mask;
			});
			return n;
		}

		template<typename T> inline component_data<T> &getList() {
			return std::get<component_data<T>>(data);
		}
		
		template<typename T> inline T &getComponent(entity ent) {
			return getList<T>()[ent.id];
		}
	};
}

#endif // __COMPONENTS_H__