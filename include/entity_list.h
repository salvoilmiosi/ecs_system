#ifndef __ENTITY_LIST_H__
#define __ENTITY_LIST_H__

#include "types.h"

namespace ecs {
	template<size_t Size = MAX_ENTITIES>
	class entity_list : public std::array<entity, Size> {
	private:
		size_t currentSize;
		size_t nextSize;

	public: 
		entity_list() {
			currentSize = 0;
			nextSize = 0;

			for(size_t i=0; i<Size; ++i) {
				(*this)[i].id = i;
			}
		}

	public:
		auto end() {
			return std::array<entity, Size>::begin() + currentSize;
		}

		entity &createEntity() {
			if (nextSize >= Size) throw std::out_of_range("Out of memory");

			// Update moves all dead entities to the right,
			// so the first entity_id in nextSize should be free

			entity &ent = (*this)[nextSize];
			ent.mask = 0;
			ent.alive = true;
			
			++nextSize;
			
			return ent;
		}

		void update() {
			//moves all alive entities to the left, all dead entities to the right
			//credit to Vittorio Romeo for the algorithm
			size_t iD = 0, iA = nextSize - 1;

			while(true) {
				for (; true; ++iD) {
					if (iD > iA) goto end_of_loop;
					if (!(*this)[iD].alive) break;
				}
				for (; true; --iA) {
					if ((*this)[iA].alive) break;
					if (iA <= iD) goto end_of_loop;
				}
				std::swap((*this)[iA], (*this)[iD]);

				++iD;
				--iA;
			}

			end_of_loop:
			currentSize = nextSize = iD;
		}
	};

	inline auto &allEntities() {
		static entity_list obj;
		return obj;
	}
}

#endif