#ifndef __CONSOLE_UI_H_
#define __CONSOLE_UI_H_

#include "console.h"

#include <SDL2/SDL_ttf.h>

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

	void render(SDL_Renderer *renderer, TTF_Font *font, int x, int y);

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

class console_ui : public console {
public:
	typedef std::function<void(const std::string &)> string_func;

	console_ui(string_func func) : parseCommandFunc(func) {}

	~console_ui() {
		lines.clear();
	}
	
	virtual void addLine(Uint32 color, const std::string &msg);

	virtual void tick() {};

	virtual void render(SDL_Renderer *renderer) = 0;

	// returns true if no events are handled
	virtual bool handleEvent(const SDL_Event &event);

protected:
	std::deque<console_line> lines;

	string_func parseCommandFunc;

	bool shown = false;

	std::string typing;
	console_line typing_line;

	Uint32 time_open;

	TTF_Font *font;
	int font_size = 16;
};

class console_dev : public console_ui {
public:
	console_dev(string_func func) : console_ui(func) {}

	virtual void render(SDL_Renderer *renderer);

	virtual bool handleEvent(const SDL_Event &event);
};

class console_chat : public console_ui {
public:
	console_chat(string_func func) : console_ui(func) {}

	virtual void tick();

	virtual void render(SDL_Renderer *renderer);

	virtual bool handleEvent(const SDL_Event &event);
};

}

#endif // __CONSOLE_UI_H__