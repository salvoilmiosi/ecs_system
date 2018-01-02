#include "console_ui.h"

#include <SDL2/SDL_ttf.h>

namespace console {

const char *FONT_NAME = "COURIER.TTF";
const int FONT_SIZE = 18;
TTF_Font *font = nullptr;

const char *BLINKER = "|";
const Uint32 BLINK_TIME = 500;

SDL_Color getColor(Uint32 color) {
	Uint8 r = (color & 0xff000000) >> (8 * 3);
	Uint8 g = (color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (color & 0x000000ff) >> (8 * 0);
	return SDL_Color{r, g, b, a};
}

void setRenderColor(SDL_Renderer *renderer, Uint32 color) {
	SDL_Color col = getColor(color);
	SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, col.a);
}

console_line::~console_line() {
	if (texture) {
		SDL_DestroyTexture(texture);
	}
}

console_line &console_line::operator = (const std::string &str) {
	if (str == line) return *this;
	if (texture) {
		SDL_DestroyTexture(texture);
		texture = nullptr;
	}
	line = str;
	return *this;
}

void console_line::render(SDL_Renderer *renderer, int x, int y) {
	if (!font) {
		font = TTF_OpenFont(FONT_NAME, FONT_SIZE);
	}
	if (!texture && !line.empty()) {
		SDL_Surface *surf = TTF_RenderText_Blended(font, line.c_str(), getColor(color));
		rect = surf->clip_rect;
		texture = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_FreeSurface(surf);
	}
	if (texture) {
		SDL_Rect dst_rect = rect;
		dst_rect.x = x;
		dst_rect.y = y;
		SDL_RenderCopy(renderer, texture, &rect, &dst_rect);
	}
}

void console_ui::addLine(Uint32 color, const std::string &msg) {
	std::lock_guard lock(l_mutex);
	lines.emplace_back(msg, color);
	if (lines.size() > MAX_LINES) {
		lines.pop_front();
	}
}

bool console_ui::handleEvent(const SDL_Event &event) {
	switch (event.type) {
	case SDL_KEYUP:
		if (event.key.repeat) break;
		if (!shown && event.key.keysym.scancode == key_to_open) {
			time_open = event.key.timestamp;
			typing = "";
			shown = true;
			SDL_StartTextInput();
		}
		break;
	case SDL_KEYDOWN:
		if (event.key.repeat) break;
		switch (event.key.keysym.scancode) {
		case SDL_SCANCODE_ESCAPE:
			if (shown) {
				SDL_StopTextInput();
			}
			shown = false;
			break;
		case SDL_SCANCODE_RETURN:
			if (shown && parseCommandFunc && !typing.empty()) {
				parseCommandFunc(typing);
				typing = "";
			}
			break;
		case SDL_SCANCODE_BACKSPACE:
			if (shown && !typing.empty()) {
				typing.pop_back();
			}
			break;
		default:
			break;
		}
		break;
	case SDL_TEXTINPUT:
		if (shown) {
			typing += event.edit.text;
		}
		break;
	default:
		break;
	}
	return !shown;
}

void console_ui::tick() {
	std::lock_guard lock(l_mutex);
	if (type == CONSOLE_CHAT) {
		if (!lines.empty() && lines.front().time_elapsed() > LINE_MAX_TIME) {
			lines.pop_front();
		}
	}
}

void console_ui::render(SDL_Renderer *renderer) {
	std::lock_guard lock(l_mutex);
	if (!shown && type == CONSOLE_DEV) return;

	SDL_Rect bg_rect{0, 0, SCREEN_W, SCREEN_H};

	if (type == CONSOLE_DEV) {
		bg_rect.h = FONT_SIZE * (MAX_LINES + 1);
	} else if (type == CONSOLE_CHAT) {
		bg_rect.h = lines.size() * FONT_SIZE;
		bg_rect.y = SCREEN_H - bg_rect.h - FONT_SIZE;
	}

	setRenderColor(renderer, COLOR_BG);
	SDL_RenderFillRect(renderer, &bg_rect);

	int x = bg_rect.x;
	int y = bg_rect.y;
	if (type == CONSOLE_CHAT) {
		y += bg_rect.h;
		for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
			y -= it->height();
			it->render(renderer, x, y);
		}
	} else {
		for (auto &line : lines) {
			line.render(renderer, x, y);
			y += line.height();
		}
	}
	
	if (!shown && type == CONSOLE_CHAT) return;

	y = bg_rect.y + bg_rect.h;
	if (type == CONSOLE_DEV) {
		y -= FONT_SIZE;
	}
	typing_line = ((SDL_GetTicks() - time_open) % (BLINK_TIME * 2) < BLINK_TIME) ? typing + BLINKER : typing;
	typing_line.render(renderer, x, y);
}

}