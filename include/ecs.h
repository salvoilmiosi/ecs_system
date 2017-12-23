#ifndef __ECS_H__
#define __ECS_H__

#include "mpl.h"
#include "growing_array.h"

#include <bitset>
#include <functional>

namespace ecs {

static const size_t MAX_ENTITIES_DEFAULT = 4096;

typedef size_t entity_id;

template<typename ... Components>
using component_list = mpl::TypeList<Components...>;

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world {
	static_assert(mpl::allHaveDefaultConstructor<ComponentList>{});

protected:
	template<typename T>
	using container = std::array<T, MaxEntities>;

	template<typename ... Ts>
	using components_tuple = std::tuple<container<Ts>...>;

	mpl::Rename<components_tuple, ComponentList> component_data;

	typedef std::bitset<ComponentList::size> component_mask;
	struct entity {
		bool alive = false;
		component_mask mask;
	};

	container<entity_id> entity_id_list;
	container<entity> entity_list;

	size_t currentSize = 0;
	size_t nextSize = 0;
	size_t maxSize = 0;
	size_t capacity = MaxEntities;

	template<typename T>
	static constexpr bool isComponent() {
		return mpl::Contains<T, ComponentList>{};
	}

	template<typename ... Ts>
	static constexpr bool areAllComponents() {
		return mpl::ContainsAll<mpl::TypeList<Ts...>, ComponentList>{};
	}

private:
	void growContainers();

	void addComponentsHelper(entity_id ent) {}

	template<typename T, typename ... Ts>
	void addComponentsHelper(entity_id ent, T first, Ts ... components) {
		addComponent(ent, first);
		addComponentsHelper(ent, components ...);
	}

public:
	template<typename ... Ts>
	constexpr static component_mask generateMask() {
		static_assert(areAllComponents<Ts ...>());
		return ((component_mask(1) << mpl::IndexOf<Ts, ComponentList>::value) | ...);
	}

	template<typename T>
	constexpr T &getComponent(entity_id ent) {
		static_assert(isComponent<T>());
		return std::get<container<T>>(component_data)[ent];
	}

	template<typename T>
	void addComponent(entity_id ent, T component) {
		static_assert(isComponent<T>());
		getComponent<T>(ent) = component;
		entity_list[ent].mask |= generateMask<T>();
	}

	template<typename ... Ts>
	void addComponents(entity_id ent, Ts ... components) {
		static_assert(areAllComponents<Ts ...>());
		addComponentsHelper(ent, components ...);
	}

	template<typename T>
	void removeComponent(entity_id ent) {
		static_assert(isComponent<T>());
		entity_list[ent].mask &= ~(generateMask<T>());
	}

	bool entityMatches(entity_id ent, const component_mask &mask) {
		return (entity_list[ent].mask & mask) == mask;
	}

	template<typename ... Ts>
	bool hasComponents(entity_id ent) {
		static_assert(areAllComponents<Ts ...>());
		return entityMatches(ent, generateMask<Ts...>());
	}

	template<typename ... Ts>
	entity_id createEntity(Ts ... components);

	void removeEntity(entity_id ent) {
		entity_list[ent].alive = false;
		entity_list[ent].mask.reset();
	}

	size_t entityCount() {
		return currentSize;
	}

	template<typename Func>
	void forEachEntity(const Func &func) {
		for (size_t i=0; i<currentSize; ++i) {
			func(entity_id_list[i]);
		}
	}

	void updateEntities();
};

template<typename ComponentList, size_t MaxEntities>
void world<ComponentList, MaxEntities>::growContainers() {
	throw std::out_of_range("Containers are full");
	/*
	entity_id_list.grow();
	entity_list.grow();

	mpl::for_each_in_tuple(component_data, [](auto &x){
		x.grow();
	});

	capacity = entity_id_list.size();*/
}

template<typename ComponentList, size_t MaxEntities> template<typename ... Ts>
entity_id world<ComponentList, MaxEntities>::createEntity(Ts ... components) {
	if (nextSize >= capacity) {
		growContainers();
	}

	// Update moves all dead entities to the right,
	// so the first entity_id in nextSize should be free

	if (nextSize >= maxSize) {
		++maxSize; // the first entity is 1
		entity_id_list[nextSize] = maxSize;
	}

	entity_id ent = entity_id_list[nextSize];
	entity_list[ent].alive = true;
	entity_list[ent].mask.reset();
	
	++nextSize;

	addComponentsHelper(ent, components ...);
	return ent;
}

template<typename ComponentList, size_t MaxEntities>
void world<ComponentList, MaxEntities>::updateEntities() {
	// Moves all alive entities to the left, all dead entities to the right
	// Credit to Vittorio Romeo for the algorithm
	if (nextSize == 0) return;
	
	size_t iD = 0, iA = nextSize - 1;

	while (true) {
		for (; true; ++iD) {
			if (iD > iA) goto end_of_loop;
			if (!entity_list[entity_id_list[iD]].alive) break;
		}
		for (; true; --iA) {
			if (entity_list[entity_id_list[iA]].alive) break;
			if (iA <= iD) goto end_of_loop;
		}
		std::swap(entity_id_list[iA], entity_id_list[iD]);

		++iD;
		--iA;
	}

	end_of_loop:
	currentSize = nextSize = iD;
}

template<typename ... Ts>
class system {
private:
	std::function<void(entity_id, Ts& ...)> func;

public:
	system(decltype(func) func) : func(func) {}

	template<typename ComponentList, size_t MaxEntities>
	void execute(world<ComponentList, MaxEntities> &wld);
};

template<typename ... Ts> template<typename ComponentList, size_t MaxEntities>
void system<Ts...>::execute(world<ComponentList, MaxEntities> &wld) {
	static auto mask = wld.template generateMask<Ts ...>();
	wld.forEachEntity([&](entity_id ent) {
		if (wld.entityMatches(ent, mask)) {
			func(ent, wld.template getComponent<Ts>(ent) ...);
		}
	});
}

}

#endif //__ECS_H__