#ifndef __ENTITIES_H__
#define __ENTITIES_H__

#include <array>

namespace ecs {
	typedef size_t entity_id;
	
	struct entity {
		entity_id id = 0;
		bool alive = false;
	};

	template<size_t Size>
	class entity_list : public std::array<entity, Size> {
	private:
		size_t currentSize;
		size_t nextSize;

	public:
		entity_list() {
			currentSize = 0;
			nextSize = 0;

			for (entity_id id = 0; id < Size; ++id) {
				(*this)[id].id = id;
			}
		}

	public:
		auto end() const {
			return this->begin() + currentSize;
		}

		entity& createEntity() {
			if (nextSize >= Size) throw std::out_of_range("Out of memory");

			// Update moves all dead entities to the right,
			// so the first entity_id in nextSize should be free

			auto &ent = (*this)[nextSize];
			ent.alive = true;
			
			++nextSize;
			
			return ent;
		}

		inline size_t getEntityCount() const {
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