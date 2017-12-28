#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <mutex>
#include <iostream>

namespace logger {

extern std::mutex log_mutex;

template<typename ... Ts>
void log(Ts ... args) {
	std::lock_guard lock(log_mutex);

	(std::cout << ... << args) << std::endl;
}

};

#endif // __LOGGER_H_