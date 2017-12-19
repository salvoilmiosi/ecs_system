#ifndef __WORLD_NET_H__
#define __WORLD_NET_H__

#include "ecs.h"

#include "edit_logger.h"

namespace ecs {

#define SUPER world<ComponentList, MaxEntities>

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world_out : public SUPER {
public:
	template<typename T>
	void addComponent(entity_id ent, T component) {
		SUPER::addComponent(ent, component);
		logComponents<T>(ent, EDIT_ADD);
	}

	template<typename ... Ts>
	void addComponents(entity_id ent, Ts ... components) {
		SUPER::addComponents(ent, components ...);
		logComponents<Ts ...>(ent, EDIT_ADD);
	}

	template<typename T>
	void removeComponent(entity_id ent) {
		SUPER::template removeComponent<T>(ent);
		logMask(ent);
	}

	template<typename ... Ts>
	void createEntity(Ts ... components) {
		entity_id ent = SUPER::createEntity(components ...);
		logComponents<Ts ...>(ent, EDIT_CREATE);
	}

	edit_logger<ComponentList> logState();

	void flushLog(std::ostream &out) {
		logger.flush(out);
	}

private:
	edit_logger<ComponentList> logger;

	template<typename ... Ts>
	void logComponents(entity_id ent, edit_type type);

	void logMask(entity_id ent);

	template<size_t ... Is>
	constexpr auto singleEntityComponents(entity_id ent, std::index_sequence<Is ...>) {
		return std::make_tuple(std::get<Is>(this->component_data)[ent] ...);
	}

	constexpr auto singleEntityComponents(entity_id ent) {
		return singleEntityComponents(ent, std::make_index_sequence<ComponentList::size>());
	}
};

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world_in : public world<ComponentList, MaxEntities> {
public:
	void applyEdits();

	void readLog(std::istream &in) {
		logger.read(in);
	}

private:
	edit_logger<ComponentList> logger;
};

template<typename ComponentList, size_t MaxEntities> template<typename ... Ts>
void world_out<ComponentList, MaxEntities>::logComponents(entity_id ent, edit_type type) {
	auto edit = logger.create();
	edit.type = type;
	edit.id = ent;
	edit.mask = SUPER::template generateMask<Ts ...>();
	edit.data = singleEntityComponents(ent);
	logger.add(edit);
}

template<typename ComponentList, size_t MaxEntities>
void world_out<ComponentList, MaxEntities>::logMask(entity_id ent) {
	auto edit = logger.create();
	edit.type = EDIT_MASK;
	edit.id = ent;
	edit.mask = SUPER::entity_list[ent].mask;
	// edit.data is unset
	logger.add(edit);
}

template<typename ComponentList, size_t MaxEntities>
edit_logger<ComponentList> world_out<ComponentList, MaxEntities>::logState() {
	decltype(logger) state_logger;
	this->forEachEntity([&](entity_id id) {
		auto edit = state_logger.create();
		edit.type = EDIT_STATE;
		edit.id = id;
		edit.mask = SUPER::entity_list[id].mask;
		edit.data = singleEntityComponents(id);

		state_logger.add(edit);
	});
	return state_logger;
}

template<typename ComponentList, size_t MaxEntities>
void world_in<ComponentList, MaxEntities>::applyEdits() {

}
// template<typename ComponentList, size_t MaxEntities>
// void world_in<ComponentList, MaxEntities>::applyEdits() {
// 	logger.forEachEdit([this](auto &edit) {
// 		bool skip_create = false;
// 		switch (edit.type) {
// 		case EDIT_MASK:
// 			entity_list[edit.id].mask = edit.mask;
// 			if (edit.mask.none()) {
// 				entity_list[edit.id].alive = false;
// 			}
// 			break;
// 		case EDIT_STATE:
// 			// Create the entity if it doesn't exist
// 			// Else just add the components
// 			if (entity_list[edit.id].alive) {
// 				entity_list[edit.id].mask.reset();
// 				skip_create = true;
// 			}
// 		// fall through
// 		case EDIT_CREATE:
// 			if (!skip_create) {
// 				while (nextSize >= capacity) {
// 					growContainers();
// 				}

// 				entity_id_list[nextSize] = edit.id;
// 				entity_list[edit.id].alive = true;
// 				entity_list[edit.id].mask.reset();

// 				++nextSize;
// 			}
// 		// fall through
// 		case EDIT_ADD:
// 		{
// 			entity_list[edit.id].mask |= edit.mask;

// 			size_t i = 0;
// 			mpl::for_each_in_2_tuples(edit.data, component_data, [&](auto &c1, auto &c2) {
// 				if (edit.mask.test(i)) {
// 					c2[edit.id] = c1;
// 				}
// 				++i;
// 			});
// 			break;
// 		}
// 		default:
// 			break;
// 		}
// 	});
// }

#undef SUPER

}

#endif