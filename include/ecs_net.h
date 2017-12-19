#ifndef __WORLD_NET_H__
#define __WORLD_NET_H__

#include "ecs.h"

#include "edit_logger.h"
#include "packet_data.h"

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

	void flushLog(packet_data_out &out) {
		logger.write(out);
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

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world_in : public world<ComponentList, MaxEntities> {
public:
	void applyEdits();

	void readLog(packet_data_in &in) {
		logger.read(in);
	}

private:
	edit_logger<ComponentList> logger;

	typename SUPER::template container<entity_id> local_ids;

	typedef typename SUPER::component_mask component_mask;

	entity_id remoteEntity(entity_id remote_id) {
		return local_ids[remote_id];
	}

	void setMask(entity_id id, component_mask mask) {
		auto &ent = SUPER::entity_list[remoteEntity(id)];
		ent.mask = mask;
		if (mask.none()) {
			ent.alive = false;
		}
	}

	template<typename ... Ts>
	entity_id createEntity(entity_id id, Ts ... components) {
		return local_ids[id] = SUPER::createEntity(components ...);
	}

	template<typename ... Ts>
	void stateEntity(entity_id id) {
		auto &ent = SUPER::entity_list[remoteEntity(id)];
		if (ent.alive) {
			ent.mask.reset();
		} else {
			createEntity(id);
		}
	}

	template<typename T>
	void addComponent(entity_id id, T component) {
		SUPER::addComponent(remoteEntity(id), component);
	}

	template<typename ... Ts>
	void addComponents(entity_id id, Ts ... components) {
		SUPER::addComponents(remoteEntity(id), components ...);
	}

	void editHelper(auto &edit) {
		mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
			component_mask c_mask = SUPER::template generateMask<typename std::remove_reference<decltype(comp)>::type> ();
			if ((edit.mask & c_mask) == c_mask) {
				addComponent(edit.id, comp);
			}
		});
	}
};

template<typename ComponentList, size_t MaxEntities>
void world_in<ComponentList, MaxEntities>::applyEdits() {
	logger.forEachEdit([this](auto &edit) {
		switch(edit.type) {
		case EDIT_MASK:
			setMask(edit.id, edit.mask);
			break;
		case EDIT_CREATE:
			createEntity(edit.id);
			editHelper(edit);
			break;
		case EDIT_STATE:
			stateEntity(edit.id);
			editHelper(edit);
			break;
		case EDIT_ADD:
			editHelper(edit);
			break;
		case EDIT_NONE:
		default:
			break;
		}
	});
}

#undef SUPER

}

#endif