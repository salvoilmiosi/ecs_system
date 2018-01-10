#ifndef __PACKET_DATA_H__
#define __PACKET_DATA_H__

#include <SDL2/SDL_net.h>

#include <vector>
#include <string>
#include <bitset>

typedef std::vector<uint8_t> packet_data;

static const size_t PACKET_SIZE = 1024 * 8;

class packet_reader {
public:
	packet_reader(const packet_data &data) : datain(data) {}

	const uint8_t *read(size_t len) {
		const uint8_t *data_ptr = datain.data() + index;
		index += len;
		return data_ptr;
	}

	bool eof() {
		return index >= datain.size();
	}

	size_t at() {
		return index;
	}

private:
	const packet_data &datain;

	size_t index = 0;

	const uint8_t *data_ptr() {
		return datain.data() + index;
	}
};

class packet_writer {
public:
	packet_writer() {
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

uint8_t readByte(packet_reader &in);
void writeByte(packet_writer &out, const uint8_t &obj);

std::string readString(packet_reader &in);
void writeString(packet_writer &out, const std::string &obj);

uint16_t readShort(packet_reader &in);
void writeShort(packet_writer &out, const uint16_t &obj);

uint32_t readLong(packet_reader &in);
void writeLong(packet_writer &out, const uint32_t &obj);

uint64_t readLongLong(packet_reader &in);
void writeLongLong(packet_writer &out, const uint64_t &obj);

float readFloat(packet_reader &in);
void writeFloat(packet_writer &out, const float &obj);

double readDouble(packet_reader &in);
void writeDouble(packet_writer &out, const double &obj);

template<size_t Size>
std::bitset<Size> readBitset(packet_reader &in) {
	if constexpr (Size <= 32) {
		return readLong(in);
	} else if constexpr (Size <= 64) {
		return readLongLong(in);
	} else {
		std::bitset<Size> bits;
		for (size_t i = 0; i < Size; i += 32) {
			bits |= std::bitset<Size>(readLong(in)) << i;
		}
		return bits;
	}
}

template<size_t Size>
void writeBitset(packet_writer &out, const std::bitset<Size> &bits) {
	if constexpr (Size <= 32)  {
		writeLong(out, bits.to_ulong());
	} else if constexpr (Size <= 64) {
		writeLongLong(out, bits.to_ullong());
	} else {
		for (size_t i = 0; i < Size; i += 32) {
			writeLong(out, ((bits >> i) & std::bitset<Size>(0xffffffff)).to_ulong());
		}
	}
}

#endif // __PACKET_DATA_H__