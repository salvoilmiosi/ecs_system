#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <cstdio>
#include <mutex>

class logger {
public:
	template<typename ... Ts>
	void log(const char *format, Ts ... args) {
		std::lock_guard lock(l_mutex);

		printf(format, args ...);	}

private:
	std::mutex l_mutex;
};

#endif