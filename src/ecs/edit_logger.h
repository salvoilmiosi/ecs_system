#ifndef __EDIT_LOGGER_H__
#define __EDIT_LOGGER_H__

#include <deque>
#include <iostream>
#include <bitset>
#include <tuple>
#include <sstream>

#include "mpl.h"
#include "packet_data.h"

namespace ecs {

enum edit_type {
	EDIT_NONE, // no edit
	EDIT_MASK, // change just the mask
	EDIT_CREATE, // create entity and add components
	EDIT_STATE, // edit all components
	EDIT_ADD, // add components in mask
};

template<typename ComponentList>
class edit_logger {
public:
	typedef size_t entity_id;
	typedef std::bitset<ComponentList::size> edit_mask;

	template<typename ... Ts>
	using component_tuple = std::tuple<Ts...>;

	typedef mpl::Rename<component_tuple, ComponentList> edit_data;

	struct entity_edit {
		edit_type type;
		entity_id id;
		edit_mask mask;
		edit_data data;
	};

	entity_edit create() {
		return entity_edit();
	}

	void add(const entity_edit &edit) {
		edits.push_back(edit);
	}

	void read(packet_reader &in);
	void write(packet_writer &out);

	// iterates over the deque while clearing it
	template<typename Func>
	void forEachEdit(Func func) {
		while (!edits.empty()) {
			auto &obj = edits.front();
			func(obj);
			edits.pop_front();
		}
	}

private:
	std::deque<entity_edit> edits;
};

template<typename ComponentList>
void edit_logger<ComponentList>::read(packet_reader &in) {
	while (! in.eof()) {
		auto edit = create();
		edit.type = static_cast<edit_type>(readByte(in));

		if (edit.type == EDIT_NONE) continue;
		
		edit.id = readLongLong(in);

		edit.mask = readLongLong(in);

		if (edit.type != EDIT_MASK) {
			mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
				using component_type = typename std::remove_reference<decltype(comp)>::type;
				if constexpr (! std::is_base_of<tag, component_type>::value) {
					auto c_mask = world<ComponentList>::template generateMask<component_type> ();
					if ((edit.mask & c_mask) == c_mask) {
						comp.read(in);
					}
				}
			});
		}

		add(edit);
	}
}

template<typename ComponentList>
void edit_logger<ComponentList>::write(packet_writer &out) {
	forEachEdit([&](auto &edit){
		writeByte(out, edit.type);

		if (edit.type == EDIT_NONE) return;

		writeLongLong(out, edit.id);
		writeLongLong(out, edit.mask.to_ullong());

		if (edit.type == EDIT_MASK) return;

		mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
			using component_type = typename std::remove_reference<decltype(comp)>::type;
			if constexpr (! std::is_base_of<tag, component_type>::value) {
				auto c_mask = world<ComponentList>::template generateMask<component_type>();
				if ((edit.mask & c_mask) == c_mask) {
					comp.write(out);
				}
			}
		});
	});
}

}

#endif // __EDIT_LOGGER_H__