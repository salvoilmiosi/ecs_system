#ifndef __ECS_H__
#define __ECS_H__

#include "components.h"
#include "entity_list.h"
#include "systems.h"

namespace ecs {
	template<typename T> inline void addComponent(entity &ent, T component) {
		// trova la lista di componenti di T e aggiunge component
		auto &cl = getList<T>();
		cl[ent.id] = component;
		ent.mask |= cl.mask();
	}

	inline void addComponents(entity ent) {}

	template<typename T, typename ... Ts> inline void addComponents(entity &ent, T first, Ts ... components) {
		addComponent(ent, first);
		addComponents(ent, components ...);
	}

	template<typename T> inline bool hasComponent(const entity &ent) {
		auto &cl = getList<T>();
		return ent.mask & cl.mask() == cl.mask();
	}

	template<typename T> inline void removeComponent(const entity &ent, T component) {
		ent.mask &= ~(getList<T>().mask());
	}

	template<typename ... T> inline entity &createEntity(T ... components) {
		entity &ent = allEntities().createEntity();

		addComponents(ent, components ...);

		return ent;
	}

	inline void removeEntity(entity &ent) {
		// basta settare a 0 la maschera
		ent.mask = 0;
		ent.alive = false;
	}

	inline void updateEntities() {
		allEntities().update();
	}
}

#endif //__ECS_H__