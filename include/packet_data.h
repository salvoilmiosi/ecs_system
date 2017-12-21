#ifndef __PACKET_DATA_H__
#define __PACKET_DATA_H__

#include <SDL2/SDL_net.h>

#include <vector>
#include <algorithm>

typedef std::vector<uint8_t> packet_data;

static const size_t PACKET_SIZE = 1024 * 8;

class packet_data_in {
public:
	packet_data_in(const packet_data &data) : data(data) {}

	const uint8_t *read(size_t len) {
		const uint8_t *data_ptr = data.data() + index;
		index += len;
		return data_ptr;
	}

	bool eof() {
		return index >= data.size();
	}

	size_t at() {
		return index;
	}

private:
	const packet_data &data;

	size_t index = 0;

	const uint8_t *data_ptr() {
		return data.data() + index;
	}
};

class packet_data_out {
public:
	packet_data_out() {
		dataout.reserve(PACKET_SIZE);
	}

	void write(const uint8_t *data, size_t len) {
		dataout.insert(dataout.end(), data, data + len);
	}

	const auto &data() {
		return dataout;
	}


private:
	packet_data dataout;
};

// expect linker error if serializer function are undefined
template<typename T> T readBinary(packet_data_in &in);

inline uint8_t readByte(packet_data_in &in) {
	return *(in.read(sizeof(uint8_t)));
}

inline std::string readString(packet_data_in &in) {
	uint8_t len = readByte(in);
	const uint8_t *begin = in.read(len);
	return std::string(begin, begin + len);
}

inline uint16_t readShort(packet_data_in &in) {
	return SDLNet_Read16(in.read(sizeof(uint16_t)));
}

inline uint32_t readLong(packet_data_in &in) {
	return SDLNet_Read32(in.read(sizeof(uint32_t)));
}

inline uint64_t readLongLong(packet_data_in &in) {
	uint64_t byte0 = readLong(in);
	uint64_t byte1 = readLong(in);
	return byte0 << 32 | byte1;
}

// expect linker error if serializer function are undefined
template<typename T> void writeBinary(packet_data_out &out, const T& obj);

inline void writeByte(packet_data_out &out, const uint8_t &obj) {
	out.write(&obj, sizeof(uint8_t));
}

inline void writeString(packet_data_out &out, const std::string &obj) {
	writeByte(out, obj.size());
	out.write(reinterpret_cast<const uint8_t *>(obj.data()), obj.size());
}

inline void writeShort(packet_data_out &out, const uint16_t &obj) {
	uint16_t pack;
	SDLNet_Write16(obj, &pack);
	out.write(reinterpret_cast<const uint8_t *>(&pack), sizeof(pack));
}

inline void writeLong(packet_data_out &out, const uint32_t &obj) {
	uint32_t pack;
	SDLNet_Write32(obj, &pack);
	out.write(reinterpret_cast<const uint8_t *>(&pack), sizeof(pack));
}

inline void writeLongLong(packet_data_out &out, const uint64_t &obj) {
	writeLong(out, (obj & 0xffffffff00000000) >> 32);
	writeLong(out, obj & 0x00000000ffffffff);
}

#define pack754_32(f) (pack754((f), 32, 8))
#define pack754_64(f) (pack754((f), 64, 11))
#define unpack754_32(i) (unpack754((i), 32, 8))
#define unpack754_64(i) (unpack754((i), 64, 11))

static uint64_t pack754(long double f, unsigned bits, unsigned expbits)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0) return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }

    // get the normalized form of f and track the exponent
    shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

    // return the final answer
    return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

static long double unpack754(uint64_t i, unsigned bits, unsigned expbits)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0) return 0.0;

    // pull the significand
    result = (i&((1LL<<significandbits)-1)); // mask
    result /= (1LL<<significandbits); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(expbits-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }

    // sign it
    result *= (i>>(bits-1))&1? -1.0: 1.0;

    return result;
}

inline float readFloat(packet_data_in &in) {
	uint32_t i = readLong(in);
	return unpack754_32(i);
}

inline double readDouble(packet_data_in &in) {
	uint64_t i = readLongLong(in);
	return unpack754_64(i);
}

inline void writeFloat(packet_data_out &out, const float &obj) {
	uint32_t i = pack754_32(obj);
	writeLong(out, i);
}

inline void writeDouble(packet_data_out &out, const double &obj) {
	uint64_t i = pack754_64(obj);
	writeLongLong(out, i);
}

#endif // __PACKET_DATA_H__