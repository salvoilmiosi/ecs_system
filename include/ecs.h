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
		template<typename Component, size_t Size>
		using component_entry = std::array<Component, Size>;

		template<typename ... Ts>
		using component_data = std::tuple<component_entry<Ts, MaxEntities>...>;

		typedef std::bitset<ComponentList::size> component_mask;

		mpl::Rename<component_data, ComponentList> data;

		entity_list<MaxEntities> ents;

		std::array<component_mask, MaxEntities> mask_list;

	public:
		template<typename T> void addComponent(entity& ent, T component) {
			static_assert(isComponent<T>());
			// trova la lista di componenti di T e aggiunge component
			getComponent<T>(ent) = component;
			getMask(ent) |= generateMask<T>();
		}

		void addComponents(entity& ent) {}

		template<typename T, typename ... Ts> inline void addComponents(entity& ent, T first, Ts ... components) {
			addComponent(ent, first);
			addComponents(ent, components ...);
		}

		template<typename T> inline bool hasComponent(entity& ent) {
			static_assert(isComponent<T>());
			auto &cl = getEntry<T>();
			return getMask(ent) & cl.mask == cl.mask;
		}

		template<typename T> inline void removeComponent(entity& ent, T component) {
			getMask(ent) &= ~(generateMask<T>());
		}

		template<typename ... T> inline entity& createEntity(T ... components) {
			entity& ent = ents.createEntity();
			getMask(ent) = 0;
			addComponents(ent, components ...);
			return ent;
		}

		template<typename T>
		constexpr auto &getEntry() {
			return std::get<component_entry<T, MaxEntities>>(data);
		}
		
		template<typename ... Ts>
		constexpr auto generateMask() {
			static_assert(areAllComponents<Ts ...>());
			return or_all(component_mask(1) << mpl::IndexOf<Ts, ComponentList>::value ...);
		}

		auto &getMask(entity& ent) {
			return mask_list[ent.id];
		}

		template<typename T>
		constexpr T &getComponent(entity& ent) {
			return getEntry<T>()[ent.id];
		}

		inline void removeEntity(entity& ent) {
			getMask(ent) = 0;
			ent.alive = false;
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