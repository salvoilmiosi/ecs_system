#ifndef __ECS_H__
#define __ECS_H__

#include "util.h"

#include <bitset>
#include <array>
#include <functional>

namespace ecs {
	static const size_t MAX_ENTITIES_DEFAULT = 4096;
	
	typedef size_t entity_id;

	template<typename ... Components>
	using component_list = mpl::TypeList<Components...>;

	template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
	class world {
	public:
		template<typename T>
		static constexpr bool isComponent() {
			return mpl::Contains<T, ComponentList>{};
		}

		template<typename ... Ts>
		static constexpr bool areAllComponents() {
			return mpl::ContainsAll<mpl::TypeList<Ts...>, ComponentList>{};
		}

	private:
		template<typename T>
		using container = std::array<T, MaxEntities>;

		container<entity_id> entity_id_list;

		typedef std::bitset<ComponentList::size> component_mask;
		struct entity {
			bool alive = false;
			component_mask mask;
		};

		container<entity> entity_list;

		template<typename ... Ts>
		using components_tuple = std::tuple<container<Ts>...>;

		static_assert(mpl::allHaveDefaultConstructor<ComponentList>{});
		mpl::Rename<components_tuple, ComponentList> component_data;

	private:
		size_t currentSize = 0;
		size_t nextSize = 0;

	public:
		world() {
			for (entity_id i=0; i<entity_id_list.size(); ++i) {
				entity_id_list[i] = i;
			}
		}

	public:
		void addComponents(entity_id ent) {}

		template<typename T> void addComponent(entity_id ent, T component) {
			static_assert(isComponent<T>());
			// trova la lista di componenti di T e aggiunge component
			getComponent<T>(ent) = component;
			entity_list[ent].mask |= generateMask<T>();
		}

		template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
			addComponent(ent, first);
			addComponents(ent, components ...);
		}

		bool entityMatches(entity_id ent, component_mask mask) {
			return (entity_list[ent].mask & mask) == mask;
		}

		template<typename ... Ts> inline bool hasComponents(entity_id ent) {
			static_assert(areAllComponents<Ts ...>());
			return entityMatches(ent, generateMask<Ts...>());
		}

		template<typename T> inline void removeComponent(entity_id ent, T component) {
			static_assert(isComponent<T>());
			entity_list[ent].mask &= ~(generateMask<T>());
		}

		template<typename ... Ts> inline entity_id createEntity(Ts ... components) {
			static_assert(areAllComponents<Ts ...>());

			if (nextSize >= MaxEntities) throw std::out_of_range("Out of memory");

			// Update moves all dead entities to the right,
			// so the first entity_id in nextSize should be free

			entity_id ent = entity_id_list[nextSize];
			entity_list[ent].alive = true;
			entity_list[ent].mask.reset();
			
			++nextSize;

			addComponents(ent, components ...);
			return ent;
		}
		
		template<typename ... Ts>
		constexpr auto generateMask() {
			static_assert(areAllComponents<Ts ...>());
			return or_all(component_mask(1) << mpl::IndexOf<Ts, ComponentList>::value ...);
		}

		template<typename T>
		constexpr T &getComponent(entity_id ent) {
			return std::get<container<T>>(component_data)[ent];
		}

		inline void removeEntity(entity_id ent) {
			entity_list[ent].alive = false;
			entity_list[ent].mask.reset();
		}

		size_t entityCount() {
			return currentSize;
		}

		void forEachEntity(auto func) {
			for (size_t i=0; i<currentSize; ++i) {
				func(entity_id_list[i]);
			}
		}

		inline void updateEntities() {
			//moves all alive entities to the left, all dead entities to the right
			//credit to Vittorio Romeo for the algorithm
			size_t iD = 0, iA = nextSize - 1;

			while(true) {
				for (; true; ++iD) {
					if (iD > iA) goto end_of_loop;
					if (!entity_list[entity_id_list[iD]].alive) break;
				}
				for (; true; --iA) {
					if (entity_list[entity_id_list[iA]].alive) break;
					if (iA <= iD) goto end_of_loop;
				}
				std::swap(entity_id_list[iA], entity_id_list[iD]);

				++iD;
				--iA;
			}

			end_of_loop:
			currentSize = nextSize = iD;
		}
	};

	template<typename ... Ts>
	class system {
	private:
		std::function<void(entity_id, Ts& ...)> func;

	public:
		system(auto _func) {
			static_assert(std::is_assignable<decltype(func), decltype(_func)>{});
			func = _func;
		}

		void execute(auto &world) {
			static_assert(world.template areAllComponents<Ts...>());
			auto mask = world.template generateMask<Ts ...>();
			world.forEachEntity([this, &world, mask](entity_id ent){
				if (world.entityMatches(ent, mask)) {
					func(ent, world.template getComponent<Ts>(ent) ...);
				}
			});
		}
	};
}

#endif //__ECS_H__