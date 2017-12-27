#ifndef __COMPONENTS_SERIAL_H__
#define __COMPONENTS_SERIAL_H__

#include "components.h"
#include "packet_data.h"

template<> inline sprite readBinary(packet_data_in &in) {
	sprite obj;
	obj.src = readString(in);
	obj.color = readLong(in);
	return obj;
}

template<> inline position readBinary(packet_data_in &in) {
	return position(readFloat(in), readFloat(in));
}

template<> inline velocity readBinary(packet_data_in &in) {
	return velocity(readFloat(in), readFloat(in));
}

template<> inline acceleration readBinary(packet_data_in &in) {
	return acceleration(readFloat(in), readFloat(in));
}

template<> inline scale readBinary(packet_data_in &in) {
	return scale(readFloat(in));
}

template<> inline shrinking readBinary(packet_data_in &in) {
	return shrinking(readFloat(in));
}

template<> inline health readBinary(packet_data_in &in) {
	return health(readLong(in));
}

template<> inline generator readBinary(packet_data_in &in) {
	return generator(readLong(in));
}

template<> inline void writeBinary(packet_data_out &out, const sprite &obj) {
	writeString(out, obj.src);
	writeLong(out, obj.color);
}

template<> inline void writeBinary(packet_data_out &out, const position &obj) {
	writeFloat(out, obj.x);
	writeFloat(out, obj.y);
}

template<> inline void writeBinary(packet_data_out &out, const velocity &obj) {
	writeFloat(out, obj.x);
	writeFloat(out, obj.y);
}

template<> inline void writeBinary(packet_data_out &out, const acceleration &obj) {
	writeFloat(out, obj.x);
	writeFloat(out, obj.y);
}

template<> inline void writeBinary(packet_data_out &out, const scale &obj) {
	writeFloat(out, obj.value);
}

template<> inline void writeBinary(packet_data_out &out, const shrinking &obj) {
	writeFloat(out, obj.value);
}

template<> inline void writeBinary(packet_data_out &out, const health &obj) {
	writeLong(out, obj.value);
}

template<> inline void writeBinary(packet_data_out &out, const generator &obj) {
	writeLong(out, obj.particles_per_tick);
}

#endif // __COMPONENTS_SERIAL_H__