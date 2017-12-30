#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <sstream>
#include <string_view>

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

std::string_view getCommand(std::string_view cmd);

std::string_view getArgument(std::string_view args, size_t i);

}

#endif // __LOGGER_H_