#include "console_ui.h"

namespace console {

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

void console_line::render(SDL_Renderer *renderer, TTF_Font *font, int x, int y) {
	if (line.empty()) return;
	if (!texture) {
		SDL_Surface *surf = TTF_RenderText_Blended(font, line.c_str(), getColor(color));
		rect = surf->clip_rect;
		texture = SDL_CreateTextureFromSurface(renderer, surf);
		SDL_FreeSurface(surf);
	}
	SDL_Rect dst_rect = rect;
	dst_rect.x = x;
	dst_rect.y = y;
	SDL_RenderCopy(renderer, texture, &rect, &dst_rect);
}

console_ui::console_ui(string_func func, console_type type) : parseCommandFunc(func), type(type) { }

void console_ui::addLine(Uint32 color, const std::string &msg) {
	std::lock_guard lock(l_mutex);
	lines.emplace_back(msg, color);
	if (lines.size() > MAX_LINES) {
		lines.pop_front();
	}
}

bool console_ui::handleEvent(const SDL_Event &event) {
	SDL_Scancode key_to_open = (type == CONSOLE_CHAT) ? SDL_SCANCODE_RETURN : SDL_SCANCODE_GRAVE;
	switch (event.type) {
	case SDL_KEYUP:
		if (!event.key.repeat && event.key.keysym.scancode == key_to_open) {
			if (shown) {
				shown = false;
			} else {
				time_open = event.key.timestamp;
				typing = "";
				shown = true;
				SDL_StartTextInput();
			}
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
	if (!font) {
		if (type == CONSOLE_DEV) {
			font = TTF_OpenFont("font/courier.ttf", font_size);
		} else {
			font = TTF_OpenFont("font/OpenSans-Regular.ttf", font_size);
		}
		if (!font) return;
	}

	std::lock_guard lock(l_mutex);
	if (!shown && type == CONSOLE_DEV) return;

	SDL_Rect bg_rect{0, 0, SCREEN_W, SCREEN_H};

	if (type == CONSOLE_DEV) {
		bg_rect.h = font_size * (MAX_LINES + 1);
	} else if (type == CONSOLE_CHAT) {
		bg_rect.h = lines.size() * font_size;
		bg_rect.y = SCREEN_H - bg_rect.h - font_size;
		if (shown) {
			bg_rect.h += font_size;
		}
	}

	setRenderColor(renderer, COLOR_BG);
	SDL_RenderFillRect(renderer, &bg_rect);

	int x = bg_rect.x;
	int y = bg_rect.y;
	if (type == CONSOLE_CHAT) {
		y += bg_rect.h;
		if (shown) {
			y -= font_size;
		}
		for (auto it = lines.rbegin(); it != lines.rend(); ++it) {
			y -= font_size;
			it->render(renderer, font, x, y);
		}
	} else {
		for (auto &line : lines) {
			line.render(renderer, font, x, y);
			y += font_size;
		}
	}
	
	if (!shown && type == CONSOLE_CHAT) return;

	y = bg_rect.y + bg_rect.h - font_size;
	typing_line = ((SDL_GetTicks() - time_open) % (BLINK_TIME * 2) < BLINK_TIME) ? typing + BLINKER : typing;
	typing_line.render(renderer, font, x, y);
}

}