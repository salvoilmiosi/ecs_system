#ifndef __ECS_H__
#define __ECS_H__

#include <map>
#include <tuple>
#include <functional>

#include "tuple_util.h"
#include "components.h"
#include "systems.h"

static entity_t maxEntityId = 0;

template<typename T> inline void addComponent(entity_t ent, T component) {
	// trova la lista di componenti di T e aggiunge component
	auto &cl = comp::getList<T>();
	cl[ent] = component;
	comp::mask_list[ent] |= cl.mask();
}

inline void addComponents(entity_t ent) {}

template<typename T, typename ... Ts> inline void addComponents(entity_t ent, T first, Ts ... components) {
	addComponent(ent, first);
	addComponents(ent, components ...);
}

template<typename T> inline bool hasComponent(entity_t ent) {
	auto &cl = comp::getList<T>();
	return cl.find(ent) != cl.end();
	// oppure controllare se matcha la maschera
}

template<typename T> inline void removeComponent(entity_t ent, T component) {
	auto &cl = comp::getList<T>();
	cl.erase(ent);
	comp::mask_list[ent] &= ~(cl.mask());
}

template<typename ... T> inline entity_t createEntity(T ... components) {
	entity_t ent = maxEntityId++;

	addComponents(ent, components ...);

	return ent;
}

inline void removeEntity(entity_t ent) {
	// per ogni component_list elimina ent
	for_each_in_tuple(comp::all, [ent](auto &x) {
		x.erase(ent);
	});
	comp::mask_list.erase(ent);
}

#endif //__ECS_H__