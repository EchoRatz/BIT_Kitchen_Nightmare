#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>

#ifndef Time
#define Time

extern Uint32 timerStartTicks; // Stores the ticks when the timer was started
extern Uint32 timerPausedTicks; // Stores the ticks when the timer was paused
extern Uint32 accumulatedTime; // Accumulated paused time
extern bool timerPaused; // Is the timer paused?
extern bool timerStarted; // Is the timer started?

void startTimer();
void stopTimer();
void pauseTimer();
void resumeTimer();
Uint32 getTimerTime();
	
#endif // !Time




