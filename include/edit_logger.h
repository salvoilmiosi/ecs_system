#ifndef __EDIT_LOGGER_H__
#define __EDIT_LOGGER_H__

#include <deque>
#include <iostream>
#include <bitset>
#include <tuple>

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

	void read(packet_data_in &in);

	// iterates over the deque while clearing it
	template<typename Func>
	void forEachEdit(Func func) {
		while (!edits.empty()) {
			auto &obj = edits.front();
			func(obj);
			edits.pop_front();
		}
	}

	void write(packet_data_out &out);

private:
	std::deque<entity_edit> edits;
};

class syntax_error : public std::invalid_argument {
public:
	syntax_error(char expected, char got, packet_data_in &in) :
		std::invalid_argument(msg(expected, got, in.at() - 1)) {}

private:
	std::string msg(char expected, char got, size_t at) {
		std::string str = "Syntax error: expected ";
		str += expected;
		str += " in position ";
		str += std::to_string(at);
		str += ", got ";
		str += got;
		return str;
	}
};

#define CHECK_CHAR(x) if (char got = readBinary<char>(in); got != x) throw syntax_error(x, got, in)

template<typename ComponentList>
void edit_logger<ComponentList>::read(packet_data_in &in) {
	while (! in.eof()) {
		auto edit = create();
		CHECK_CHAR('T');
		edit.type = static_cast<edit_type>(readBinary<uint8_t>(in));

		if (edit.type == EDIT_NONE) continue;
		CHECK_CHAR('I');
		
		edit.id = readBinary<uint64_t>(in);

		CHECK_CHAR('M');
		edit.mask = readBinary<uint64_t>(in);

		if (edit.type != EDIT_MASK) {
			mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
				using component_type = typename std::remove_reference<decltype(comp)>::type;
				auto c_mask = world<ComponentList>::template generateMask<component_type> ();
				if ((edit.mask & c_mask) == c_mask) {
					CHECK_CHAR('C');
					comp = readBinary<component_type>(in);
				}
			});
		}

		add(edit);
	}
}

#undef CHECK_CHAR

template<typename ComponentList>
void edit_logger<ComponentList>::write(packet_data_out &out) {
	forEachEdit([&](auto &edit){
		writeBinary(out, 'T');
		writeBinary<uint8_t>(out, edit.type);

		if (edit.type == EDIT_NONE) return;

		writeBinary(out, 'I');
		writeBinary<uint64_t>(out, edit.id);
		writeBinary(out, 'M');
		writeBinary<uint64_t>(out, edit.mask.to_ullong());

		if (edit.type == EDIT_MASK) return;

		mpl::for_each_in_tuple(edit.data, [&](auto &comp) {
			using component_type = typename std::remove_reference<decltype(comp)>::type;
			auto c_mask = world<ComponentList>::template generateMask<component_type>();
			if ((edit.mask & c_mask) == c_mask) {
				writeBinary(out, 'C');
				writeBinary(out, comp);
			}
		});
	});
}

}

#endif // __EDIT_LOGGER_H__