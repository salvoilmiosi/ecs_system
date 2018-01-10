#include "game_client.h"

#include <SDL2/SDL.h>

#include "SDL_draw.h"

#include <map>

namespace game {

game_client::game_client(console::console_dev &console_dev) :
	console_dev(console_dev),
	console_chat([&](const std::string &str) {
		sock.sendMessage(str);
	}) {}

void game_client::start() {

}

void game_client::listen() {
	sock.forEachPacket([&](auto &x) {
		packet_reader in(x);
		net::packet_type type = static_cast<net::packet_type>(readByte(in));
		switch (type) {
		case net::PACKET_EDITLOG:
			in_logger.read(in);
			break;
		case net::PACKET_SERVER_MSG:
			console_chat.addLine(console::COLOR_LOG, console::format("Server: ", readString(in)));
			break;
		case net::PACKET_SERVER_CHAT:
		{
			std::string name = readString(in);
			std::string message = readString(in);
			console_chat.addLine(console::COLOR_DEFAULT, console::format(name, " : ", message));
			break;
		}
		case net::PACKET_SERVER_QUIT:
			console_chat.addLine(console::COLOR_LOG, "Server has quit.");
			disconnect();
			break;
		case net::PACKET_NONE:
		default:
			break;
		}
	});

	wld.applyEdits(in_logger);
}

void game_client::tick() {
	wld.executeSystem<position, velocity>([&](ecs::entity_id id, position &pos, velocity &vel) {
		pos.value += vel.value;
	});
	wld.executeSystem<velocity, acceleration>([&](ecs::entity_id id, velocity &vel, acceleration &acc) {
		vel.value += acc.value;
	});
	wld.executeSystem<rotation, rotation_accel>([&](ecs::entity_id id, rotation &rot, rotation_accel &ra) {
		rot.value += ra.value;
	});
	wld.executeSystem<velocity, position, screen_bounce>([&](ecs::entity_id id, velocity &vel, position &pos) {
		if ((vel.value.x > 0 && pos.value.x > SCREEN_W) ||
				(vel.value.x < 0 && pos.value.x < 0)) vel.value.x *= -1.f;
		if ((vel.value.y > 0 && pos.value.y > SCREEN_H) ||
				(vel.value.y < 0 && pos.value.y < 0)) vel.value.y *= -1.f;
	});
	wld.executeSystem<scale, shrinking>([&](ecs::entity_id id, scale &sca, shrinking &shr) {
		sca.value *= shr.value;
	});
	wld.executeSystem<circle, collision, position, velocity, scale>([&](ecs::entity_id ball_a, position&, velocity&, scale&) {
		wld.executeSystem<circle, collision, position, velocity, scale>([&](ecs::entity_id ball_b, position&, velocity&, scale&) {
			if (ball_a != ball_b && collides(ball_a, ball_b)) {
				resolveCollision(ball_a, ball_b);
			}
		});
	});
	wld.executeSystem<health, dying>([&](ecs::entity_id me, health &hp) {
		--hp.value;
	});
	wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
		if (hp.value <= 0) {
			wld.removeEntity(me);
		}
	});

	wld.executeSystem<position, generator>([&](ecs::entity_id id, position &pos) {
		generateParticles(pos);
	});

	wld.updateEntities();

	console_chat.tick();
}

void game_client::render(SDL_Renderer *renderer) {
	wld.executeSystem<square, color, position, scale>([&](ecs::entity_id id, color &col, position &pos, scale &sca) {
		renderSquare(renderer, id, col, pos, sca);
	});
	wld.executeSystem<circle, color, position, scale>([&](ecs::entity_id id, color &col, position &pos, scale &sca) {
		renderCircle(renderer, id, col, pos, sca);
	});

	console_chat.render(renderer);
}

void game_client::handleEvent(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		close();
		return;
	}

	if (! console_dev.handleEvent(event)) return;
	if (! console_chat.handleEvent(event)) return;

	switch (event.type) {
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEMOTION:
	case SDL_MOUSEBUTTONUP:
		sock.sendInputCommand(userinput::handleEvent(event));
		break;
	default:
		break;
	}
}

bool game_client::command(const std::string &full_cmd) {
	std::string_view cmd = console::getCommand(full_cmd);
	if (cmd == "quit") {
		close();
	} else if (cmd == "disconnect") {
		disconnect();
	} else if (cmd == "say") {
		sock.sendMessage(std::string(console::getArgs(full_cmd)));
	} else if (cmd == "state") {
		sock.sendStatePacket();
	} else if (cmd == "name") {
		username = console::getArgument(full_cmd, 1);
		//sock.sendUsernamePacket(username);
	} else if (cmd == "connect") {
		connect(std::string(console::getArgument(full_cmd, 1)));
	} else {
		return false;
	}
	return true;
}

void game_client::generateParticles(position &pos) {
	for (int i=0; i<5; ++i) {
		Uint8 r = rand() % 0xff;
		Uint8 g = rand() % 0xff;
		Uint8 b = rand() % 0xff;
		Uint8 a = rand() % 0xff;
		color color_random((r << (8 * 3)) | (g << (8 * 2)) | (b << (8 * 1)) | (a << (8 * 0)));
		
		position position_random(
			pos.value.x + ((float) rand() / RAND_MAX - 0.5f) * 100.f,
			pos.value.y + ((float) rand() / RAND_MAX - 0.5f) * 100.f);

		velocity velocity_random(
			((float) rand() / RAND_MAX - 0.5f) * 8.f,
			((float) rand() / RAND_MAX - 0.5f) * 8.f);

		acceleration acceleration_random(
			((float) rand() / RAND_MAX - 0.5f) * 0.2f,
			((float) rand() / RAND_MAX - 0.5f) * 0.2f);

		try {
			wld.createEntity(position_random, color_random, velocity_random, acceleration_random,
				scale(rand() % 15 + 25.f), shrinking(0.983f), health(rand() % 100 + 50), circle());
		} catch (std::out_of_range) {
			// out of memory
			return;
		}
	}
}

static const int SPRITE_W = 512;
static const int SPRITE_H = 512;

class texture_holder {
public:
	texture_holder(SDL_Renderer *renderer);
	~texture_holder() {
		if (surface) SDL_FreeSurface(surface);
		if (texture) SDL_DestroyTexture(texture);
	}

	void draw(SDL_Renderer *renderer, SDL_Rect *rect, Uint32 color, float rotation = 0.f);

protected:
	SDL_Surface *surface = nullptr;
	SDL_Texture *texture = nullptr;
};

texture_holder::texture_holder(SDL_Renderer *renderer) {
	surface = SDL_CreateRGBSurface(0, SPRITE_W, SPRITE_H, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
}

void texture_holder::draw(SDL_Renderer *renderer, SDL_Rect *rect, Uint32 color, float rotation) {
	SDL_QueryTexture(texture, nullptr, nullptr, &surface->w, &surface->h);

	Uint8 r = (color & 0xff000000) >> (8 * 3);
	Uint8 g = (color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (color & 0x000000ff) >> (8 * 0);
	SDL_SetTextureColorMod(texture, r, g, b);
	SDL_SetTextureAlphaMod(texture, a);
	if (rotation == 0.f) {
		SDL_RenderCopy(renderer, texture, &surface->clip_rect, rect);
	} else {
		SDL_RenderCopyEx(renderer, texture, &surface->clip_rect, rect, rotation, nullptr, SDL_FLIP_NONE);
	}
}

class square : public texture_holder {
public:
	square(SDL_Renderer *renderer);
};

square::square(SDL_Renderer *renderer) : texture_holder(renderer) {
	SDL_FillRect(surface, &surface->clip_rect, 0xffffffff);
	texture = SDL_CreateTextureFromSurface(renderer, surface);
}

class circle : public texture_holder {
public:
	circle(SDL_Renderer *renderer);
};

circle::circle(SDL_Renderer *renderer) : texture_holder(renderer) {
	SDL_FillRect(surface, &surface->clip_rect, 0x0);
	Draw_FillCircle(surface, surface->w / 2, surface->h / 2, ((surface->w < surface->h) ? surface->w : surface->h) / 2, 0xffffffff);

	texture = SDL_CreateTextureFromSurface(renderer, surface);
}

void game_client::renderSquare(SDL_Renderer *renderer, ecs::entity_id ent, color &col, position &pos, scale &s) {
	static square texture(renderer);

	SDL_Rect rect {(int)(pos.value.x - s.value * 0.5f), (int)(pos.value.y - s.value * 0.5f), (int)s.value, (int)s.value};

	if (wld.hasComponent<rotation>(ent)) {
		texture.draw(renderer, &rect, col.value, wld.getComponent<rotation>(ent).value);
	} else {
		texture.draw(renderer, &rect, col.value);
	}
}

void game_client::renderCircle(SDL_Renderer *renderer, ecs::entity_id ent, color &col, position &pos, scale &s) {
	static circle texture(renderer);

	SDL_Rect rect {(int)(pos.value.x - s.value * 0.5f), (int)(pos.value.y - s.value * 0.5f), (int)s.value, (int)s.value};

	texture.draw(renderer, &rect, col.value);
}

bool game_client::collides(ecs::entity_id ball_a, ecs::entity_id ball_b) {
	auto &pos_a = wld.getComponent<position>(ball_a);
	auto &pos_b = wld.getComponent<position>(ball_b);
	float dx = pos_a.value.x - pos_b.value.x;
	float dy = pos_a.value.y - pos_b.value.y;
	float rad_sum = (wld.getComponent<scale>(ball_a).value + wld.getComponent<scale>(ball_b).value) * 0.5f;
	return dx * dx + dy * dy < rad_sum * rad_sum;
}

void game_client::resolveCollision(ecs::entity_id ball_a, ecs::entity_id ball_b) {
	vector2d delta = wld.getComponent<position>(ball_a).value - wld.getComponent<position>(ball_b).value;
	double d = delta.length();

	vector2d mtd = delta * (((wld.getComponent<scale>(ball_a).value + wld.getComponent<scale>(ball_b).value) * 0.5f - d) / d);

	double im1 = wld.getComponent<scale>(ball_a).value * wld.getComponent<scale>(ball_a).value;
	double im2 = wld.getComponent<scale>(ball_b).value * wld.getComponent<scale>(ball_b).value;

	if (d == 0) {
		double angle = rand() / (double)RAND_MAX * M_PI;
		vector2d moveV(cos(angle), sin(angle));
		wld.getComponent<position>(ball_a).value += moveV * wld.getComponent<scale>(ball_a).value * 0.5f;
		wld.getComponent<position>(ball_b).value -= moveV * wld.getComponent<scale>(ball_b).value * 0.5f;
		return;
	}

	wld.getComponent<position>(ball_a).value += mtd * (im1 / (im1 + im2));
	wld.getComponent<position>(ball_b).value -= mtd * (im2 / (im1 + im2));

	vector2d v = wld.getComponent<velocity>(ball_a).value - wld.getComponent<velocity>(ball_b).value;

	if (v.length()<0.1f) {
		return;
	}

	vector2d normMtd = mtd.normalize();
	double vn = v.normalize().dot(normMtd);
	if (vn > 0) return;

	double i =  -vn / (im1+im2);
	vector2d impulse = mtd * i;

	wld.getComponent<velocity>(ball_a).value += impulse * im1;
	wld.getComponent<velocity>(ball_b).value -= impulse * im2;
}


}