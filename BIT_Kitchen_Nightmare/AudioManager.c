#include "AudioManager.h"
#include <SDL_mixer.h>
#include <stdio.h>
#include <SDL.h>

//Static global pointer to the background music object
static Mix_Music* bgMusic = NULL;

//Static global array of pointers to sound effect objects
static Mix_Chunk* soundEffects[SOUND_COUNT];

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

void AudioManager_LoadAndPlayMusic(const char* file_path, int loop) {
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

	if (Mix_PlayMusic(currentMusic, loop) == -1) {
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

void AudioManager_LoadPredefinedSoundEffect() {
	soundEffects[SOUND_SLASH] = Mix_LoadWAV("Assets/SoundEffects/Slash_Effect.wav");
	soundEffects[SOUND_CLICK] = Mix_LoadWAV("Assets/SoundEffects/Click_Effect.wav");
}

static int LoadSoundEffect(const char* file_path) {
	Mix_Chunk* effect = Mix_LoadWAV(file_path); //Load the sound effect from the specified file path
	if (effect == NULL) {
		printf("Failed to load sound effect! SDL_mixer Error: %s\n", Mix_GetError());
		return -1;
	}
	return effect;
}

void AudioManager_PlayEffect(SoundEffect sound) {
	if(sound >= 0 && sound < SOUND_COUNT) {
		Mix_PlayChannel(-1, soundEffects[sound], 0); //Play the sound effect
	}
	else {
		printf("Invalid sound effect identifier: %d\n", sound);
	}
}

void AudioManager_Cleanup() {
	//Free the music object and shut down SDL_mixer
	if (bgMusic != NULL) {
		Mix_FreeMusic(bgMusic);
		bgMusic = NULL;
	}
	for (int i = 0; i < SOUND_COUNT; ++i) {
		Mix_FreeChunk(soundEffects[i]);
	}
	Mix_CloseAudio();
}