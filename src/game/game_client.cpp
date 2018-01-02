#include "game_client.h"

namespace game {

game_client::game_client(console::console_ui &console_dev) :
	console_dev(console_dev),
	console_chat([&](const std::string &str) {
		sock.sendMessage(str);
	}, console::CONSOLE_CHAT) {}

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
			console_chat.addLine(console::COLOR_LOG, "Server has quit");
			sock.close();
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
		pos.value.x += vel.value.x;
		pos.value.y += vel.value.y;
	});
	wld.executeSystem<velocity, acceleration>([&](ecs::entity_id id, velocity &vel, acceleration &acc) {
		vel.value.x += acc.value.x;
		vel.value.y += acc.value.y;
	});
	wld.executeSystem<scale, shrinking>([&](ecs::entity_id id, scale &sca, shrinking &shr) {
		sca.value *= shr.value;
	});
	wld.executeSystem<health>([&](ecs::entity_id me, health &hp) {
		--hp.value;
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
	wld.executeSystem<sprite, position, scale>([&](ecs::entity_id id, sprite &spr, position &pos, scale &sca) {
		renderEntity(renderer, spr, pos, sca);
	});

	console_chat.render(renderer);
}

void game_client::handleEvent(const SDL_Event &event) {
	if (event.type == SDL_QUIT) {
		sock.close();
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
		return true;
	} else if (cmd == "say") {
		sock.sendMessage(std::string(console::getArgs(full_cmd)));
		return true;
	} else if (cmd == "state") {
		sock.sendStatePacket();
		return true;
	}
	return false;
}

void game_client::generateParticles(position &pos) {
	for (int i=0; i<5; ++i) {
		Uint8 r = rand() % 0xff;
		Uint8 g = rand() % 0xff;
		Uint8 b = rand() % 0xff;
		Uint8 a = rand() % 0xff;
		sprite sprite_random((r << (8 * 3)) | (g << (8 * 2)) | (b << (8 * 1)) | (a << (8 * 0)));
		
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
			wld.createEntity(position_random, sprite_random, velocity_random, acceleration_random,
				scale(rand() % 15 + 25.f), shrinking(0.983f), health(rand() % 100 + 50));
			// will create on average 2000 entities
		} catch (std::out_of_range) {
			// out of memory
			return;
		}
	}
}

void game_client::renderEntity(SDL_Renderer *renderer, sprite &spr, position &pos, scale &s) {
	SDL_Rect rect {(int)(pos.value.x - s.value * 0.5f), (int)(pos.value.y - s.value * 0.5f), (int)s.value, (int)s.value};
	Uint8 r = (spr.color & 0xff000000) >> (8 * 3);
	Uint8 g = (spr.color & 0x00ff0000) >> (8 * 2);
	Uint8 b = (spr.color & 0x0000ff00) >> (8 * 1);
	Uint8 a = (spr.color & 0x000000ff) >> (8 * 0);
	SDL_SetRenderDrawColor(renderer, r, g, b, a);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
	SDL_RenderFillRect(renderer, &rect);
}

}