#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <SDL2/SDL.h>

#include <iostream>
#include <sstream>
#include <string_view>
#include <deque>
#include <mutex>

static const int SCREEN_W = 1024;
static const int SCREEN_H = 768;

namespace console {

static const Uint32 COLOR_BG = 0x00008080;
static const Uint32 COLOR_DEFAULT = 0xffffffff;
static const Uint32 COLOR_ERROR = 0xff0000ff;
static const Uint32 COLOR_LOG = 0xffff00ff;

template<typename ... Ts>
std::string format(Ts&& ... args) {
	std::ostringstream str;
	(str << ... << args);
	return str.str();
}

std::string_view getCommand(std::string_view cmd);

std::string_view getArgument(std::string_view args, size_t i);

std::string_view getArgs(std::string_view full_cmd);

class console {
public:
	virtual void addLine(Uint32 color, const std::string &msg) {
		std::lock_guard lock(l_mutex);
		std::cout << msg << std::endl;
	}

protected:
	std::mutex l_mutex;
};

}

#endif // __LOGGER_H_