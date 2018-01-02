#ifndef __CONSOLE_UI_H_
#define __CONSOLE_UI_H_

#include "console.h"

#include <functional>

namespace console {

static const int MAX_LINES = 8;
static const int LINE_MAX_TIME = 5000;

class console_line {
public:
	console_line() {}

	console_line(const std::string &line, Uint32 color) : line(line), color(color) {
		time = SDL_GetTicks();
	}
	~console_line();

	console_line &operator = (const std::string &str);

	void render(SDL_Renderer *renderer, int x, int y);

	int width() {
		return rect.w;
	}

	int height() {
		return rect.h;
	}

	Uint32 time_elapsed() {
		return SDL_GetTicks() - time;
	}
private:
	std::string line;
	Uint32 time;
	Uint32 color = COLOR_DEFAULT;
	SDL_Texture *texture = nullptr;
	SDL_Rect rect;
};

enum console_type {
	CONSOLE_DEV,
	CONSOLE_CHAT
};

class console_ui : public console {
public:
	typedef std::function<void(const std::string &)> string_func;

	console_ui(string_func func, console_type type) : parseCommandFunc(func), type(type) {
		if (type == CONSOLE_DEV) {
			key_to_open = SDL_SCANCODE_GRAVE;
		} else if (type == CONSOLE_CHAT) {
			key_to_open = SDL_SCANCODE_RETURN;
		}
	}

	~console_ui() {
		lines.clear();
	}
	
	virtual void addLine(Uint32 color, const std::string &msg);

	void tick();

	void render(SDL_Renderer *renderer);

	// returns true if no events are handled
	bool handleEvent(const SDL_Event &event);

private:
	std::deque<console_line> lines;

	string_func parseCommandFunc;

	bool shown = false;

	std::string typing;
	console_line typing_line;

	Uint32 time_open;

	console_type type;

	SDL_Scancode key_to_open;
};

}

#endif // __CONSOLE_UI_H__