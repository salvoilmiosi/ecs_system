#ifndef __TIMER_H
#define __TIMER_H

#include <SDL2/SDL.h>

class timer {
private:
	//The clock time when the timer started
	int startTicks;

	//The ticks stored when the timer was paused
	int pausedTicks;

	//The timer status
	bool paused;
	bool started;

public:
	//Initializes variables
	timer() {
		startTicks = 0;
		pausedTicks = 0;
		paused = false;
		started = false;
	}

	//The various clock actions
	void start() {
		started = true;
		paused = false;

		startTicks = SDL_GetTicks();
	}

	void stop() {
		started = false;
		paused = false;
	}
	void pause() {
		if (started && !paused) {
			paused = true;

			pausedTicks = SDL_GetTicks() - startTicks;
		}
	}
	void unpause() {
		if (paused) {
			paused = false;

			startTicks = SDL_GetTicks() - pausedTicks;

			pausedTicks = 0;
		}
	}

	//Gets the timer's time
	int get_ticks() {
		if (started) {
			if (paused) {
				return pausedTicks;
			} else {
				return SDL_GetTicks() - startTicks;
			}
		}

		return 0;
	}

	//Checks the status of the timer
	bool is_started() { return started; }
	bool is_paused() { return paused; }
};

#endif // __TIMER_H
