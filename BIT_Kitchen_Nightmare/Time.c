#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include <Time.h>

//In game timer
Uint32 timerStartTicks = 0; // Stores the ticks when the timer was started
Uint32 timerPausedTicks = 0; // Stores the ticks when the timer was paused
Uint32 accumulatedTime = 0; // Accumulated paused time
bool timerPaused = false; // Is the timer paused?
bool timerStarted = false; // Is the timer started?

//Timer function
void startTimer() {
	timerStarted = true;
	timerPaused = false;
	timerStartTicks = SDL_GetTicks();
	timerPausedTicks = 0;
}

void stopTimer() {
	timerStarted = false;
	timerPaused = false;
	timerStartTicks = 0;
	timerPausedTicks = 0;
	accumulatedTime = 0;
}

void pauseTimer() {
	if (timerStarted && !timerPaused) {
		timerPaused = true;
		timerPausedTicks = SDL_GetTicks() - timerStartTicks;
	}
}

void resumeTimer() {
	if (timerStarted && timerPaused) {
		timerPaused = false;
		timerStartTicks = SDL_GetTicks() - timerPausedTicks;
		timerPausedTicks = 0;
	}
}

Uint32 getTimerTime() {
	Uint32 time = 0;
	if (timerStarted) {
		if (timerPaused) {
			time = timerPausedTicks;
		}
		else {
			time = SDL_GetTicks() - timerStartTicks;
		}
	}
	return time + accumulatedTime;
}

