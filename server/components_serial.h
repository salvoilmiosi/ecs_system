#ifndef __COMPONENTS_SERIAL_H__
#define __COMPONENTS_SERIAL_H__

#include "components.h"
#include "packet_data.h"

template<> inline printable readBinary(packet_data_in &in) {
	return printable(readBinary<std::string>(in));
}

template<> inline sprite readBinary(packet_data_in &in) {
	sprite obj;
	obj.src = readBinary<std::string>(in);
	obj.color = readBinary<uint32_t>(in);
	return obj;
}

template<> inline position readBinary(packet_data_in &in) {
	return position(readBinary<float>(in), readBinary<float>(in));
}

template<> inline velocity readBinary(packet_data_in &in) {
	return velocity(readBinary<float>(in), readBinary<float>(in));
}

template<> inline acceleration readBinary(packet_data_in &in) {
	return acceleration(readBinary<float>(in), readBinary<float>(in));
}

template<> inline scale readBinary(packet_data_in &in) {
	return scale(readBinary<float>(in));
}

template<> inline shrinking readBinary(packet_data_in &in) {
	return shrinking(readBinary<float>(in));
}

template<> inline health readBinary(packet_data_in &in) {
	return health(readBinary<uint32_t>(in));
}

template<> inline generator readBinary(packet_data_in &in) {
	return generator(readBinary<uint32_t>(in));
}

template<> inline void writeBinary(packet_data_out &out, const printable &obj) {
	writeBinary<std::string>(out, obj.name);
}

template<> inline void writeBinary(packet_data_out &out, const sprite &obj) {
	writeBinary<std::string>(out, obj.src);
	writeBinary<uint32_t>(out, obj.color);
}

template<> inline void writeBinary(packet_data_out &out, const position &obj) {
	writeBinary<float>(out, obj.x);
	writeBinary<float>(out, obj.y);
}

template<> inline void writeBinary(packet_data_out &out, const velocity &obj) {
	writeBinary<float>(out, obj.x);
	writeBinary<float>(out, obj.y);
}

template<> inline void writeBinary(packet_data_out &out, const acceleration &obj) {
	writeBinary<float>(out, obj.x);
	writeBinary<float>(out, obj.y);
}

template<> inline void writeBinary(packet_data_out &out, const scale &obj) {
	writeBinary<float>(out, obj.value);
}

template<> inline void writeBinary(packet_data_out &out, const shrinking &obj) {
	writeBinary<float>(out, obj.value);
}

template<> inline void writeBinary(packet_data_out &out, const health &obj) {
	writeBinary<uint32_t>(out, obj.value);
}

template<> inline void writeBinary(packet_data_out &out, const generator &obj) {
	writeBinary<uint32_t>(out, obj.particles_per_tick);
}

#endif // __COMPONENTS_SERIAL_H__