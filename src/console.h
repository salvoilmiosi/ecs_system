#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <mutex>
#include <iostream>
#include <sstream>

namespace console {

template<typename ... Ts>
std::string format(Ts&& ... args) {
	std::ostringstream str;
	(str << ... << args);
	return str.str();
}

class logger {
public:
	logger(std::ostream &out = std::cout) : out(out) {}

	template<typename ... Ts>
	void operator() (Ts&& ... args) {
		std::lock_guard lock(log_mutex);

		(out << ... << args) << std::endl;
	}

private:
	std::ostream &out;

	std::mutex log_mutex;
};

extern logger log;

}

#endif // __LOGGER_H_