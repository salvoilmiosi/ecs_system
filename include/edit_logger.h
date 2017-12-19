#ifndef __EDIT_LOGGER_H__
#define __EDIT_LOGGER_H__

#include <deque>
#include <iostream>
#include <bitset>
#include <tuple>

#include "mpl.h"

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

	void read(std::istream &in) {
		while (!in.eof()) {
			auto edit = create();
			readBinary<char>(in); // 'T'
			edit.type = static_cast<edit_type>(readBinary<uint8_t>(in));

			if (edit.type == EDIT_NONE) continue;

			readBinary<char>(in); // 'I'
			readBinary<uint64_t>(edit.id, in);

			readBinary<char>(in); // 'M'
			edit.mask = readBinary<uint64_t>(in);

			if (edit.type == EDIT_MASK) continue;

			size_t i = 0;
			mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
				if (edit.mask.test(i)) {
					readBinary<char>(in); // 'C'
					readBinary(comp, in);
				}
				++i;
			});

			add(edit);
		}
	}

	// iterates over the deque while clearing it
	template<typename Func>
	void forEachEdit(Func func) {
		while (!edits.empty()) {
			auto &obj = edits.front();
			if (obj.type != EDIT_NONE) {
				func(obj);
			}
			edits.pop_front();
		}
	}

	void flush(std::ostream &out) {
		forEachEdit([&](auto &obj){
			writeBinary('T', out);
			writeBinary<uint8_t>(obj.type, out);
			writeBinary('I', out);
			writeBinary<uint64_t>(obj.id, out);
			writeBinary('M', out);
			writeBinary<uint64_t>(obj.mask.to_ullong(), out);

			if (obj.type != EDIT_MASK) {
				size_t i = 0;
				mpl::for_each_in_tuple(obj.data, [&](auto &comp) {
					if (obj.mask.test(i)) {
						writeBinary('C', out);
						writeBinary(comp, out);
					}
					++i;
				});
			}
		});
	}

private:
	std::deque<entity_edit> edits;

	template<typename T>
	void writeBinary(const T& obj, std::ostream &out) {
		out.write(reinterpret_cast<const char *>(&obj), sizeof(T));
	}

	template<typename T>
	void readBinary(T &obj, std::istream &in) {
		in.read(reinterpret_cast<char *>(&obj), sizeof(T));
	}

	template<typename T>
	T readBinary(std::istream &in) {
		T obj;
		readBinary(obj, in);
		return obj;
	}
};

}

#endif // __EDIT_LOGGER_H__