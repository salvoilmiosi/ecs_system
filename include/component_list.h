#ifndef __COMPONENT_LIST_H__
#define __COMPONENT_LIST_H__

#include "types.h"

namespace ecs {
	inline component_mask &nextMask() {
		static component_mask obj = 1;
		return obj <<= 1;
	}

	template<typename T, size_t Size = MAX_ENTITIES>
	class component_list : public std::array<T, Size> {
	private:
		const component_mask i_mask;

	public:
		component_list() : std::array<T, Size>(), i_mask(nextMask()) {}

		component_mask mask() {
			return i_mask;
		}
	};

}

#endif //__COMPONENT_LIST_H__