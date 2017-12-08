#ifndef __ECS_H__
#define __ECS_H__

#include "component_list.h"

namespace ecs {
	template<typename Components>
	class manager {
	private:
		Components comp;

		entity_list<> ents;

	public:
		manager() {}

	public:
		template<typename T> void addComponent(entity &ent, T component) {
			// trova la lista di componenti di T e aggiunge component
			auto &cl = comp.template getList<T>();
			cl[ent.id] = component;
			ent.mask |= cl.mask;
		}

		void addComponents(entity ent) {}

		template<typename T, typename ... Ts> inline void addComponents(entity &ent, T first, Ts ... components) {
			addComponent(ent, first);
			addComponents(ent, components ...);
		}

		template<typename T> inline bool hasComponent(const entity &ent) {
			auto &cl = comp.template getList<T>();
			return ent.mask & cl.mask == cl.mask;
		}

		template<typename T> inline void removeComponent(const entity &ent, T component) {
			ent.mask &= ~(comp.template getList<T>().mask);
		}

		template<typename ... T> inline entity &createEntity(T ... components) {
			entity &ent = ents.createEntity();

			addComponents(ent, components ...);

			return ent;
		}

		inline void removeEntity(entity &ent) {
			// basta settare a 0 la maschera
			ent.mask = 0;
			ent.alive = false;
		}

		inline void updateEntities() {
			ents.update();
		}

		template<typename System>
		void executeSystem(System sys) {
			sys.execute(comp, ents);
		}
	};
}

#endif //__ECS_H__