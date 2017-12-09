#ifndef __ECS_H__
#define __ECS_H__

#include "types.h"

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
		using component_data = std::tuple<component_entry<Ts, MaxEntities>...>;

		mpl::Rename<component_data, ComponentList> data;

		entity_list<ComponentList::size, MaxEntities> ents;

	public:
		world() {}

		template<typename T> void addComponent(entity_id ent, T component) {
			static_assert(isComponent<T>());
			// trova la lista di componenti di T e aggiunge component
			getComponent<T>(ent) = component;
			ents.findEntity(ent).mask |= generateMask<T>();
		}

		void addComponents(entity_id ent) {}

		template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
			addComponent(ent, first);
			addComponents(ent, components ...);
		}

		template<typename T> inline bool hasComponent(entity_id ent) {
			static_assert(isComponent<T>());
			auto &cl = getEntry<T>();
			return ents.findEntity(ent).mask & cl.mask == cl.mask;
		}

		template<typename T> inline void removeComponent(entity_id ent, T component) {
			ents.findEntity(ent).mask &= ~(generateMask<T>());
		}

		template<typename ... T> inline entity_id createEntity(T ... components) {
			entity_id ent = ents.createEntity();
			addComponents(ent, components ...);
			return ent;
		}

		template<typename T>
		constexpr auto &getEntry() {
			return std::get<component_entry<T, MaxEntities>>(data);
		}
		
		template<typename ... Ts>
		constexpr component_mask<ComponentList::size> generateMask() {
			return or_all(1 << mpl::IndexOf<Ts, ComponentList>::value ...);
		}

		template<typename T>
		constexpr T &getComponent(entity_id ent) {
			return getEntry<T>()[ent];
		}

		inline void removeEntity(entity_id id) {
			auto &ent = ents.findEntity(id);
			ent.mask = 0;
			ent.alive = false;
		}

		inline size_t getEntityCount() {
			return ents.getEntityCount();
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