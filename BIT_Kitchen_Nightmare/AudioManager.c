#include "AudioManager.h"
#include <SDL_mixer.h>
#include <stdio.h>
#include <SDL.h>

//Static global pointer to the background music object
static Mix_Music* bgMusic = NULL;

void AudioManager_Init() {
	if(SDL_Init(SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return -1;
	}

	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
		return -1;
	}
}

void AudioManager_LoadAndPlayMusic(const char* file_path) {
	//First stop and free any previous music
	if (Mix_PlayingMusic()) {
		Mix_HaltMusic();
	}
	static Mix_Music* currentMusic = NULL;
	if(currentMusic != NULL) {
		Mix_FreeMusic(currentMusic);
		currentMusic = NULL;
	}

	//Load music from the specified file path
	currentMusic = Mix_LoadMUS(file_path);
	if (currentMusic == NULL) {
		printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
		return;
	}

	Mix_PlayMusic(currentMusic, -1); //Play music infinitely

	if (!bgMusic) {
		printf("Failed to play music! SDL_mixer Error: %s\n", Mix_GetError());
	}
}

void AudioManager_PauseMusic() {
	Mix_PauseMusic(); //Pause the currently playing music
}

void AudioManager_ResumeMusic() {
	Mix_ResumeMusic(); //Resume the currently playing music
}

void AudioManager_StopMusic() {
	Mix_HaltMusic(); //Stop the currently playing music
}

void AudioManager_Cleanup() {
	//Free the music object and shut down SDL_mixer
	if (bgMusic != NULL) {
		Mix_FreeMusic(bgMusic);
		bgMusic = NULL;
	}
	Mix_CloseAudio();
}