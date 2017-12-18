#ifndef __EDIT_LOGGER_H__
#define __EDIT_LOGGER_H__

#include <deque>
#include <ostream>
#include <bitset>
#include <tuple>

#include "mpl.h"

namespace ecs {

enum edit_type {
	EDIT_NONE, // no edit, just change the mask
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

	void flush(std::ostream &out) {
		while (!edits.empty()) {
			auto &obj = edits.front();
			edits.pop_front();
			if (obj.mask.none()) continue;

			writeBinary('A', out);
			writeBinary(obj.id, out);

			writeBinary('B', out);
			writeBinary(obj.mask.to_ulong(), out);

			writeBinary('C', out);
			writeBinary(obj.type, out);

			if (obj.type == EDIT_NONE) continue;

			writeBinary('D', out);
			size_t i = 0;
			mpl::for_each_in_tuple(obj.data, [&](auto &comp){
				if (obj.mask.test(i)) {
					writeBinary(comp, out);
				}
				++i;
			});
		}
	}

private:
	std::deque<entity_edit> edits;

	template<typename T>
	void writeBinary(const T& obj, std::ostream &out) {
		out.write(reinterpret_cast<const char *>(&obj), sizeof(T));
	}
};

}

#endif // __EDIT_LOGGER_H__