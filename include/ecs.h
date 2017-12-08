#ifndef __ECS_H__
#define __ECS_H__

#include "types.h"

namespace ecs {
	template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
	class world {
	private:
		ComponentList comp;

		typename ComponentList::template ComponentData<MaxEntities> data {comp.template createComponentData<MaxEntities>()}; // SO UGLY

		entity_list<ComponentList::size(), MaxEntities> ents;

	public:
		template<typename T> void addComponent(entity_id ent, T component) {
			// trova la lista di componenti di T e aggiunge component
			auto &cl = getList<T>();
			cl[ent] = component;
			ents.findEntity(ent).mask |= cl.mask;
		}

		void addComponents(entity_id ent) {}

		template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
			addComponent(ent, first);
			addComponents(ent, components ...);
		}

		template<typename T> inline bool hasComponent(entity_id ent) {
			auto &cl = getList<T>();
			return ents.findEntity(ent).mask & cl.mask == cl.mask;
		}

		template<typename T> inline void removeComponent(entity_id ent, T component) {
			ents.findEntity(ent).mask &= ~(getList<T>().mask);
		}

		template<typename ... T> inline entity_id createEntity(T ... components) {
			entity_id ent = ents.createEntity();
			addComponents(ent, components ...);
			return ent;
		}

		template<typename T>
		constexpr auto &getList() {
			return std::get<component_entry<T, ComponentList::size(), MaxEntities>>(data);
		}
		
		template<typename ... Ts>
		constexpr auto generateMask() {
			return or_all(getList<Ts>().mask ...);
		}

		template<typename T>
		constexpr T &getComponent(entity_id ent) {
			return getList<T>()[ent];
		}

		inline void removeEntity(entity_id id) {
			auto &ent = ents.findEntity(id);
			ent.mask = 0;
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