#ifndef __ECS_H__
#define __ECS_H__

#include "systems.h"

namespace ecs {
	template<typename T> inline void addComponent(entity_id ent, T component) {
		if (ent >= MAX_ENTITIES) return;

		// trova la lista di componenti di T e aggiunge component
		auto &cl = getList<T>();
		cl[ent] = component;
		mask_list()[ent] |= cl.mask();
	}

	inline void addComponents(entity_id ent) {}

	template<typename T, typename ... Ts> inline void addComponents(entity_id ent, T first, Ts ... components) {
		addComponent(ent, first);
		addComponents(ent, components ...);
	}

	template<typename T> inline bool hasComponent(entity_id ent) {
		if (ent >= MAX_ENTITIES) return false;

		auto &cl = getList<T>();
		return mask_list()[ent] & cl.mask() == cl.mask();
	}

	template<typename T> inline void removeComponent(entity_id ent, T component) {
		if (ent >= MAX_ENTITIES) return;

		auto &cl = getList<T>();
		mask_list()[ent] &= ~(cl.mask());
	}

	template<typename ... T> inline entity_id createEntity(T ... components) {
		try {
			entity_id ent = getNextId();

			addComponents(ent, components ...);

			return ent;
		} catch (std::out_of_range) {
			return -1;
		}
	}

	inline void removeEntity(entity_id ent) {
		if (ent >= MAX_ENTITIES) return;
		
		// basta settare a 0 la maschera
		mask_list()[ent] = 0;
	}

	inline void executeAllSystems() {
		for_each_in_tuple(allSystems(), [](auto &x){
			x.execute();
		});
	}
}

#endif //__ECS_H__