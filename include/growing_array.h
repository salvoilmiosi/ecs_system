#ifndef __GROWING_ARRAY_H__
#define __GROWING_ARRAY_H__

#include <vector>

static const size_t BUFFER_SIZE_DEFAULT = 512;

template<typename T, size_t BufferSize = BUFFER_SIZE_DEFAULT>
class growing_array : public std::vector<T> {
public:
	growing_array() : std::vector<T>(BufferSize) {}

public:
	T &operator[](size_t i) {
		size_t oldSize = std::vector<T>::size();
		if (i >= oldSize) {
			size_t newSize = (oldSize / BufferSize + 1) * BufferSize;
			std::vector<T>::resize(newSize, T());
		}
		return std::vector<T>::operator[](i);
	}
};

#endif // __GROWING_ARRAY_H__