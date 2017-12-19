#ifndef __PACKET_DATA_H__
#define __PACKET_DATA_H__

#include <vector>
#include <algorithm>

typedef std::vector<uint8_t> packet_data;

static const size_t PACKET_SIZE = 1024 * 8;

class packet_data_in {
public:
	packet_data_in(const packet_data &data) : data(data) {
		data_ptr = 0;
	}

	void read(void *data_out, size_t len) {
		std::copy(data.begin() + data_ptr, data.begin() + data_ptr + len, reinterpret_cast<uint8_t *>(data_out));
		data_ptr += len;
	}

	bool eof() {
		return data_ptr >= data.size();
	}

	size_t at() {
		return data_ptr;
	}

private:
	packet_data data;

	size_t data_ptr;
};

class packet_data_out {
public:
	packet_data_out() {
		dataout.reserve(PACKET_SIZE);
	}

	void write(const void *data_in, size_t len) {
		dataout.insert(dataout.end(), reinterpret_cast<const uint8_t *>(data_in), reinterpret_cast<const uint8_t *>(data_in) + len);
	}

	const auto &data() {
		return dataout;
	}

private:
	packet_data dataout;
};

#endif // __PACKET_DATA_H__