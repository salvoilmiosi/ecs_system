#ifndef __ECS_H__
#define __ECS_H__

#include "systems.h"

namespace ecs {
	template<typename T> inline void addComponent(entity_id ent, T component) {
		// trova la lista di componenti di T e aggiunge component
		auto &cl = components::getList<T>();
		cl[ent] = component;
		if (mask_list.find(ent) == mask_list.end()) {
			mask_list[ent] = cl.mask();
		} else {
			mask_list[ent] |= cl.mask();
		}
	}

	inline void addComponents(entity_id ent) {}

	template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
		addComponent(ent, first);
		addComponents(ent, components ...);
	}

	template<typename T> inline bool hasComponent(entity_id ent) {
		auto &cl = components::getList<T>();
		return cl.find(ent) != cl.end();
		// oppure controllare se matcha la maschera
	}

	template<typename T> inline void removeComponent(entity_id ent, T component) {
		auto &cl = components::getList<T>();
		cl.erase(ent);
		mask_list[ent] &= ~(cl.mask());
	}

	template<typename ... T> inline entity_id createEntity(T ... components) {
		entity_id ent = ++maxEntityId; // should create a function to manage ids

		addComponents(ent, components ...);

		return ent;
	}

	inline void removeEntity(entity_id ent) {
		// per ogni component_list elimina ent
		for_each_in_tuple(components::all(), [ent](auto &x) {
			x.erase(ent);
		});
		mask_list[ent] = 0;
		mask_list.erase(ent);
	}

	inline void executeAllSystems() {
		for_each_in_tuple(systems::all(), [](auto &x){
			x.execute();
		});
	}
}

#endif //__ECS_H__