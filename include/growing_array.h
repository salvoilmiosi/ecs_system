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

public:
	T &operator[](size_t i) {
		size_t oldSize = std::vector<T>::size();
		if (i >= oldSize) {
			size_t newSize = oldSize + BUFFER_SIZE;
			std::vector<T>::resize(newSize);
		}
		return std::vector<T>::operator[](i);
	}
};

#endif // __GROWING_ARRAY_H__