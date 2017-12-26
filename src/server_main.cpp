#include <SDL2/SDL.h>
#include <SDL2/SDL_net.h>

#include "server_game.h"
#include "timer.h"

struct thread_wrapper {
	std::thread thread;

	template<typename Func, typename ... Args>
	thread_wrapper(Func &&func, Args&& ... args) {
		thread = std::thread(func, args ...);
	}

	~thread_wrapper() {
		if (thread.joinable()) {
			thread.join();
		}
	}
};

int main (int argc, char** argv) {
	if (SDL_Init(SDL_INIT_TIMER) == -1)
		return 1;

	if (SDLNet_Init() == -1)
		return 2;

	server::game game;

	if (!game.open())
		return 3;

	thread_wrapper cmdline_thread([&]() {
		std::string line;
		while (game.is_open()) {
			std::getline(std::cin, line);

			game.command(line);
		}
	});

	timer fps;

	while(game.is_open()) {
		fps.start();

		game.tick();

		if (fps.get_ticks() < 1000 / socket::TICKRATE) {
			SDL_Delay(1000 / socket::TICKRATE - fps.get_ticks());
		}
	}

	game.close();

	SDLNet_Quit();
	SDL_Quit();
	return 0;
}