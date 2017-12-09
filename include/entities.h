#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include <bitset>
#include <array>

namespace ecs {
	typedef size_t entity_id;

	template<size_t CompNum> using component_mask = std::bitset<CompNum>;
	
	template<size_t CompNum>
	struct entity {
		entity_id id;
		component_mask<CompNum> mask;
		bool alive;

		entity() {
			id = 0;
			mask = 0;
			alive = false;
		}
	};

	template<size_t CompNum, size_t Size>
	class entity_list : public std::array<entity<CompNum>, Size> {
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
			return std::array<entity<CompNum>, Size>::begin() + currentSize;
		}

		auto &findEntity(entity_id id) {
			for (size_t i=0; i<nextSize; ++i) {
				auto &ent = (*this)[i];
				if (ent.id == id) {
					return ent;
				}
			}
			throw std::out_of_range("Entity not found");
		}

		entity_id createEntity() {
			if (nextSize >= Size) throw std::out_of_range("Out of memory");

			// Update moves all dead entities to the right,
			// so the first entity_id in nextSize should be free

			auto &ent = (*this)[nextSize];
			ent.mask = 0;
			ent.alive = true;
			
			++nextSize;
			
			return ent.id;
		}

		inline size_t getEntityCount() {
			return currentSize;
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
}

#endif // __ENTITIES_H__