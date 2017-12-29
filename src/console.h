#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <iostream>
#include <sstream>

namespace console {

template<typename ... Ts>
std::string format(Ts&& ... args) {
	std::ostringstream str;
	(str << ... << args);
	return str.str();
}

template<typename ... Ts>
void addLine(Ts&& ... args) {
	std::cout << format(args...) << std::endl;
}

}

#endif // __LOGGER_H_