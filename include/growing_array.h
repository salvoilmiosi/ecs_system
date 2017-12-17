#ifndef __GROWING_ARRAY_H__
#define __GROWING_ARRAY_H__

#include <vector>

static const size_t BUFFER_SIZE = 512;
static const size_t RESERVED_SIZE_DEFAULT = 2048;

template<typename T, size_t ReservedSize = RESERVED_SIZE_DEFAULT>
class growing_array : public std::vector<T> {
public:
	growing_array() : std::vector<T>(BUFFER_SIZE) {
		std::vector<T>::reserve(ReservedSize);
	}

	void grow() {
		std::vector<T>::resize(std::vector<T>::size() + BUFFER_SIZE);
	}
};

#endif // __GROWING_ARRAY_H__