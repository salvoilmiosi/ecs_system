#include "components.h"

void int_component::read(packet_reader &in) {
	value = readLong(in);
}

void int_component::write(packet_writer &out) const {
	writeLong(out, value);
}

void float_component::read(packet_reader &in) {
	value = readFloat(in);
}

void float_component::write(packet_writer &out) const {
	writeFloat(out, value);
}

void vec_component::read(packet_reader &in) {
	value.x = readFloat(in);
	value.y = readFloat(in);
}

void vec_component::write(packet_writer &out) const {
	writeFloat(out, value.x);
	writeFloat(out, value.y);
}