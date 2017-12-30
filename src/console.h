#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <sstream>

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;

namespace console {

template<typename ... Ts>
std::string format(Ts&& ... args) {
	std::ostringstream str;
	(str << ... << args);
	return str.str();
}

template<typename ... Ts>
void addLine(Ts&& ... args) {
	(std::cout << ... << args) << std::endl;
}

}

#endif // __LOGGER_H_