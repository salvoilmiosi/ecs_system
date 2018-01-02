#include "game/game_server.h"

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

	console::console console_cout;

	game::game_server server(console_cout);

	if (!server.open())
		return 3;

	thread_wrapper cmdline_thread([&]() {
		std::string line;
		while (server.is_open()) {
			std::getline(std::cin, line);

			if (!line.empty() && !server.command(line)) {
				console_cout.addLine(console::COLOR_ERROR, console::format(line, " is not a valid command"));
			}
		}
	});

	timer fps;

	server.start();

	while (server.is_open()) {
		fps.start();

		server.tick();

		server.broadcast();

		if (fps.get_ticks() < 1000 / net::TICKRATE) {
			SDL_Delay(1000 / net::TICKRATE - fps.get_ticks());
		}
	}

	server.close();

	SDLNet_Quit();
	SDL_Quit();
	return 0;
}