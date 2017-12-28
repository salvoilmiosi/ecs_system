#include "components.h"

#include "ecs/packet_data.h"

template<> sprite readBinary(packet_reader &in) {
	sprite obj;
	obj.src = readString(in);
	obj.color = readLong(in);
	return obj;
}

template<> position readBinary(packet_reader &in) {
	return position(readFloat(in), readFloat(in));
}

template<> velocity readBinary(packet_reader &in) {
	return velocity(readFloat(in), readFloat(in));
}

template<> acceleration readBinary(packet_reader &in) {
	return acceleration(readFloat(in), readFloat(in));
}

template<> scale readBinary(packet_reader &in) {
	return scale(readFloat(in));
}

template<> shrinking readBinary(packet_reader &in) {
	return shrinking(readFloat(in));
}

template<> health readBinary(packet_reader &in) {
	return health(readLong(in));
}

template<> generator readBinary(packet_reader &in) {
	return generator(readLong(in));
}

template<> void writeBinary(packet_writer &out, const sprite &obj) {
	writeString(out, obj.src);
	writeLong(out, obj.color);
}

template<> void writeBinary(packet_writer &out, const position &obj) {
	writeFloat(out, obj.x);
	writeFloat(out, obj.y);
}

template<> void writeBinary(packet_writer &out, const velocity &obj) {
	writeFloat(out, obj.x);
	writeFloat(out, obj.y);
}

template<> void writeBinary(packet_writer &out, const acceleration &obj) {
	writeFloat(out, obj.x);
	writeFloat(out, obj.y);
}

template<> void writeBinary(packet_writer &out, const scale &obj) {
	writeFloat(out, obj.value);
}

template<> void writeBinary(packet_writer &out, const shrinking &obj) {
	writeFloat(out, obj.value);
}

template<> void writeBinary(packet_writer &out, const health &obj) {
	writeLong(out, obj.value);
}

template<> void writeBinary(packet_writer &out, const generator &obj) {
	writeLong(out, obj.particles_per_tick);
}