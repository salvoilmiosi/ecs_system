#ifndef __WORLD_IO_H__
#define __WORLD_IO_H__

#include "world.h"

#include "edit_logger.h"

namespace ecs {

enum world_flags {
	none = 0,
	in = 1 << 0,
	out = 1 << 1,
	inout = in | out,
};

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world_io : public world<ComponentList, MaxEntities> {
public:
	world_io(world_flags flags = world_flags::inout) : flags(flags) {}

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
	entity_id createEntity(Ts ... components) {
		entity_id ent = SUPER::createEntity(components ...);
		logComponents<Ts ...>(ent, EDIT_CREATE);
		return ent;
	}

	void removeEntity(entity_id ent) {
		SUPER::removeEntity(ent);
		logMask(ent);
	}

	edit_logger<ComponentList> createStateLogger();

	void flushLog(packet_writer &out) {
		logger.write(out);
	}

	void applyEdits(edit_logger<ComponentList> &logger);

private:
	typedef world<ComponentList, MaxEntities> SUPER;
	typedef typename SUPER::component_mask component_mask;

	edit_logger<ComponentList> logger;

	const world_flags flags;

	typename SUPER::template container<entity_id> remote_ids;

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

	entity_id remoteEntity(entity_id id) {
		return remote_ids[id];
	}

	void setMaskRemote(entity_id id, component_mask mask) {
		auto &ent = SUPER::entity_list[remoteEntity(id)];
		ent.mask = mask;
		if (mask.none()) {
			ent.alive = false;
		}
	}

	template<typename ... Ts>
	entity_id createEntityRemote(entity_id id, Ts ... components) {
		return remote_ids[id] = SUPER::createEntity(components ...);
	}

	template<typename ... Ts>
	void stateEntityRemote(entity_id id) {
		auto &ent = SUPER::entity_list[remoteEntity(id)];
		if (ent.alive) {
			ent.mask.reset();
		} else {
			createEntityRemote(id);
		}
	}

	void editHelper(auto &edit) {
		mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
			component_mask c_mask = SUPER::template generateMask<typename std::remove_reference<decltype(comp)>::type> ();
			if ((edit.mask & c_mask) == c_mask) {
				SUPER::addComponent(remoteEntity(edit.id), comp);
			}
		});
	}
};

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world_in : public world_io<ComponentList, MaxEntities> {
public:
	world_in() : world_io<ComponentList, MaxEntities>(world_flags::in) {}
};

template<typename ComponentList, size_t MaxEntities = MAX_ENTITIES_DEFAULT>
class world_out : public world_io<ComponentList, MaxEntities> {
public:
	world_out() : world_io<ComponentList, MaxEntities>(world_flags::out) {}
};

template<typename ComponentList, size_t MaxEntities> template<typename ... Ts>
void world_io<ComponentList, MaxEntities>::logComponents(entity_id ent, edit_type type) {
	if (!(flags & world_flags::out)) return;

	auto edit = logger.create();
	edit.type = type;
	edit.id = ent;
	edit.mask = SUPER::template generateMask<Ts ...>();
	edit.data = singleEntityComponents(ent);
	logger.add(edit);
}

template<typename ComponentList, size_t MaxEntities>
void world_io<ComponentList, MaxEntities>::logMask(entity_id ent) {
	if (!(flags & world_flags::out)) return;

	auto edit = logger.create();
	edit.type = EDIT_MASK;
	edit.id = ent;
	edit.mask = SUPER::entity_list[ent].mask;
	// edit.data is unset
	logger.add(edit);
}

template<typename ComponentList, size_t MaxEntities>
edit_logger<ComponentList> world_io<ComponentList, MaxEntities>::createStateLogger() {
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
void world_io<ComponentList, MaxEntities>::applyEdits(edit_logger<ComponentList> &logger) {
	if (!(flags & world_flags::in)) return;

	logger.forEachEdit([this](auto &edit) {
		switch(edit.type) {
		case EDIT_MASK:
			setMaskRemote(edit.id, edit.mask);
			break;
		case EDIT_CREATE:
			createEntityRemote(edit.id);
			editHelper(edit);
			break;
		case EDIT_STATE:
			stateEntityRemote(edit.id);
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

}

#endif // __WORLD_IO_H__