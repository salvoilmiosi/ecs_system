#ifndef __COMPONENT_LIST_H__
#define __COMPONENT_LIST_H__

#include "types.h"

static const size_t MAX_ENTITIES = 4096;

namespace ecs {
	inline component_mask &maxMask() {
		static component_mask obj = 1;
		return obj;
	}

	template<typename T, size_t Size = MAX_ENTITIES> class component_list : public std::array<T, Size> {
	private:
		const component_mask i_mask;

	public:
		component_list() : std::array<T, Size>(), i_mask(maxMask()) {
			maxMask() <<= 1;
		}

		component_mask mask() {
			return i_mask;
		}
	};

	inline auto &mask_list() {
		static component_list<component_mask> obj;
		return obj;
	}

	inline entity_id getNextId() {
		static entity_id maxEntityId = 0;
		for (size_t i=0; i<MAX_ENTITIES; ++i) {
			++maxEntityId;
			if (mask_list()[maxEntityId] == 0) {
				return maxEntityId;
			}
			if (maxEntityId >= MAX_ENTITIES) {
				maxEntityId = 0;
			}
		}
		std::cout << "OUT OF MEMORY" << std::endl;
		throw std::out_of_range("Too many entities");
	}

}

#endif //__COMPONENT_LIST_H__