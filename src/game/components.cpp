#include "components.h"

template<>
void component<int>::read(packet_reader &in) {
	value = readLong(in);
}

template<>
void component<int>::write(packet_writer &out) const {
	writeLong(out, value);
}

template<>
void component<float>::read(packet_reader &in) {
	value = readFloat(in);
}

template<>
void component<float>::write(packet_writer &out) const {
	writeFloat(out, value);
}

template<>
void component<double>::read(packet_reader &in) {
	value = readDouble(in);
}

template<>
void component<double>::write(packet_writer &out) const {
	writeDouble(out, value);
}

template<>
void component<vector2d>::read(packet_reader &in) {
	value.x = readDouble(in);
	value.y = readDouble(in);
}

template<>
void component<vector2d>::write(packet_writer &out) const {
	writeDouble(out, value.x);
	writeDouble(out, value.y);
}