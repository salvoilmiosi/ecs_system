#ifndef __ECS_H__
#define __ECS_H__

#include "mpl.h"
#include "growing_array.h"

#include "edit_logger.h"

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

private:
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

	edit_logger<ComponentList> logger;

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

	template<typename T>
	constexpr T or_all(const T &obj) {
		return obj;
	}

	template<typename T, typename ... Ts>
	constexpr T or_all(const T &first, const Ts& ... then) {
		return first | or_all(then ...);
	}

	void growContainers();

	template<size_t ... Is>
	constexpr auto singleEntityComponents(entity_id ent, std::index_sequence<Is ...>) {
		return std::make_tuple(std::get<Is>(component_data)[ent] ...);
	}

	constexpr auto singleEntityComponents(entity_id ent) {
		return singleEntityComponents(ent, std::make_index_sequence<ComponentList::size>());
	}

	template<typename ... Ts>
	void logComponents(entity_id ent, edit_type type);

	void logMask(entity_id ent);

	template<typename T>
	void addComponentHelper(entity_id ent, T component) {
		getComponent<T>(ent) = component;
		entity_list[ent].mask |= generateMask<T>();
	}

	void addComponentsHelper(entity_id ent) {}

	template<typename T, typename ... Ts>
	void addComponentsHelper(entity_id ent, T first, Ts ... components) {
		addComponentHelper(ent, first);
		addComponentsHelper(ent, components ...);
	}

public:
	explicit world(bool input = false) : logger(input) {}

	template<typename ... Ts>
	constexpr component_mask generateMask() {
		static_assert(areAllComponents<Ts ...>());
		return or_all(component_mask(1) << mpl::IndexOf<Ts, ComponentList>::value ...);
	}

	template<typename T>
	constexpr T &getComponent(entity_id ent) {
		static_assert(isComponent<T>());
		return std::get<container<T>>(component_data)[ent];
	}

	template<typename T>
	void addComponent(entity_id ent, T component) {
		static_assert(isComponent<T>());
		addComponentHelper(ent, component);
		logComponents<T>(ent, EDIT_ADD);
	}

	template<typename ... Ts>
	void addComponents(entity_id ent, Ts ... components) {
		static_assert(areAllComponents<Ts ...>());
		addComponentsHelper(ent, components ...);
		logComponents<Ts ...>(ent, EDIT_ADD);
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
		logMask(ent);
	}

	size_t entityCount() {
		return currentSize;
	}

	void forEachEntity(auto func) {
		for (size_t i=0; i<currentSize; ++i) {
			func(entity_id_list[i]);
		}
	}

	void updateEntities();

	void logState();

	void applyEdits();

	void readLog(std::istream &in) {
		logger.read(in);
	}

	void flushLog(std::ostream &out) {
		logger.flush(out);
	}
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
		entity_id_list[nextSize] = logger.input ? maxSize << 1 : maxSize; // use even ids for local entities
		++maxSize;
	}

	entity_id ent = entity_id_list[nextSize];
	entity_list[ent].alive = true;
	entity_list[ent].mask.reset();
	
	++nextSize;

	addComponentsHelper(ent, components ...);
	logComponents<Ts ...>(ent, EDIT_CREATE);
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

template<typename ComponentList, size_t MaxEntities> template<typename ... Ts>
void world<ComponentList, MaxEntities>::logComponents(entity_id ent, edit_type type) {
	if (logger.input) return;

	auto edit = logger.create();
	edit.type = type;
	edit.id = ent;
	edit.mask = generateMask<Ts ...>();
	edit.data = singleEntityComponents(ent);
	logger.add(edit);
}

template<typename ComponentList, size_t MaxEntities>
void world<ComponentList, MaxEntities>::logMask(entity_id ent) {
	if (logger.input) return;

	auto edit = logger.create();
	edit.type = EDIT_MASK;
	edit.id = ent;
	edit.mask = entity_list[ent].mask;
	// edit.data is unset
	logger.add(edit);
}

template<typename ComponentList, size_t MaxEntities>
void world<ComponentList, MaxEntities>::logState() {
	if (logger.input) return;

	forEachEntity([this](entity_id id) {
		auto edit = logger.create();
		edit.type = EDIT_STATE;
		edit.id = id;
		edit.mask = entity_list[id].mask;
		edit.data = singleEntityComponents(id);

		logger.add(edit);
	});
}

template<typename ComponentList, size_t MaxEntities>
void world<ComponentList, MaxEntities>::applyEdits() {
	logger.forEachEdit([this](auto &edit) {
		switch (edit.type) {
		case EDIT_MASK:
			entity_list[edit.id].mask = edit.mask;
			break;
		case EDIT_CREATE:
		{
			while (edit.id >= capacity) {
				growContainers();
			}
			if (edit.id >= maxSize) {
				maxSize = edit.id;
			}

			entity_id_list[nextSize] = edit.id;
			entity_list[edit.id].alive = true;

			++nextSize;
			// fall through
		}
		case EDIT_STATE:
			entity_list[edit.id].mask.reset();
			// fall through
		case EDIT_ADD:
		{
			entity_list[edit.id].mask |= edit.mask;

			size_t i = 0;
			// mpl::for_each_in_2_tuples(edit.data, component_data, [&](auto &c1, auto &c2) {
			// 	if (edit.mask.test(i)) {
			// 		c2[edit.id] = c1;
			// 	}
			// 	++i;
			// });
			break;
		}
		default:
			break;
		}
		if (edit.mask.none()) {
			entity_list[edit.id].alive = false;
		}
	});
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