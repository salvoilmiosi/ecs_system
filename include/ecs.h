#ifndef __ECS_H__
#define __ECS_H__

#include "systems.h"

namespace ecs {
	template<typename T> inline void addComponent(entity_id ent, T component) {
		if (ent >= MAX_ENTITIES) return;

		// trova la lista di componenti di T e aggiunge component
		auto &cl = components::getList<T>();
		cl[ent] = component;
		components::mask_list()[ent] |= cl.mask();
	}

	inline void addComponents(entity_id ent) {}

	template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
		addComponent(ent, first);
		addComponents(ent, components ...);
	}

	template<typename T> inline bool hasComponent(entity_id ent) {
		if (ent >= MAX_ENTITIES) return false;

		auto &cl = components::getList<T>();
		return components::mask_list()[ent] & cl.mask() == cl.mask();
	}

	template<typename T> inline void removeComponent(entity_id ent, T component) {
		if (ent >= MAX_ENTITIES) return;

		auto &cl = components::getList<T>();
		cl.erase(ent);
		components::mask_list()[ent] &= ~(cl.mask());
	}

	template<typename ... T> inline entity_id createEntity(T ... components) {
		try {
			entity_id ent = components::getNextId();

			addComponents(ent, components ...);

			return ent;
		} catch (std::out_of_range) {
			return -1;
		}
	}

	inline void removeEntity(entity_id ent) {
		if (ent >= MAX_ENTITIES) return;
		
		// per ogni component_list elimina ent
		for_each_in_tuple(components::all(), [ent](auto &x) {
			x.erase(ent);
		});
		components::mask_list()[ent] = 0;
		components::mask_list().erase(ent);
	}

	inline void executeAllSystems() {
		for_each_in_tuple(systems::all(), [](auto &x){
			x.execute();
		});
	}
}

#endif //__ECS_H__