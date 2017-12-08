#ifndef __COMPONENT_LIST_H__
#define __COMPONENT_LIST_H__

#include "types.h"

namespace ecs {
	template<typename T, size_t Size = MAX_ENTITIES>
	class component_data : public std::array<T, Size> {
	public:
		const component_mask mask;

		component_data() : mask(getMask<T>()) {}
	};

	template<typename ... Components>
	class component_list {
	private:
		constexpr auto createComponentLists() {
			return std::make_tuple(component_data<Components>()...);
		}

		std::tuple<component_data<Components>...> data = createComponentLists();

	public:
		template<typename T> inline component_data<T> &getList() {
			return std::get<component_data<T>>(data);
		}
		
		template<typename T> inline T &getComponent(entity ent) {
			return getList<T>()[ent.id];
		}
	};
}

#endif // __COMPONENTS_H__