#ifndef __ECS_H__
#define __ECS_H__

#include "types.h"

#include <bitset>

namespace ecs {
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
		template<typename ... Ts>
		using components_tuple = std::tuple<std::array<Ts, MaxEntities>...>;

		typedef std::bitset<ComponentList::size> component_mask;

		mpl::Rename<components_tuple, ComponentList> component_data;

		entity_list<MaxEntities> ents;

		std::array<component_mask, MaxEntities> mask_list;

	public:
		void addComponents(entity_id ent) {}

		template<typename T> void addComponent(entity_id ent, T component) {
			static_assert(isComponent<T>());
			// trova la lista di componenti di T e aggiunge component
			getComponent<T>(ent) = component;
			getMask(ent) |= generateMask<T>();
		}

		template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
			addComponent(ent, first);
			addComponents(ent, components ...);
		}

		template<typename T> inline bool hasComponent(entity_id ent) {
			static_assert(isComponent<T>());
			constexpr component_mask c_mask = generateMask<T>();
			return getMask(ent) & c_mask == c_mask;
		}

		template<typename T> inline void removeComponent(entity_id ent, T component) {
			getMask(ent) &= ~(generateMask<T>());
		}

		template<typename ... T> inline entity_id createEntity(T ... components) {
			entity_id ent = ents.createEntity();
			getMask(ent).reset();
			addComponents(ent, components ...);
			return ent;
		}
		
		template<typename ... Ts>
		constexpr auto generateMask() {
			static_assert(areAllComponents<Ts ...>());
			return or_all(component_mask(1) << mpl::IndexOf<Ts, ComponentList>::value ...);
		}

		component_mask &getMask(entity_id ent) {
			return mask_list[ent];
		}

		template<typename T>
		constexpr T &getComponent(entity_id ent) {
			return std::get<std::array<T, MaxEntities>>(component_data)[ent];
		}

		inline void removeEntity(entity_id ent) {
			getMask(ent).reset();
			ents.removeEntity(ent);
		}

		inline void updateEntities() {
			ents.update();
		}

		template<typename System>
		void executeSystem(System sys) {
			sys.execute(*this, ents);
		}
	};
}

#endif //__ECS_H__