#include "AudioManager.h"
#include <SDL_mixer.h>
#include <stdio.h>

//Static global pointer to the background music object
static Mix_Music* bgMusic = NULL;

void AudioManager_Init() {
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}
}

int AudioManager_LoadMusic(const char* file_path) {
	//Ensure any pervious music is stopped before loading new music
	if (bgMusic != NULL) {
		Mix_FreeMusic(bgMusic);
		bgMusic = NULL;
	}

	//Load music from the specified file path
	bgMusic = Mix_LoadMUS(file_path);
	if (bgMusic == NULL) {
		printf("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
			return 0; //Failure
	}

	return 1; //Success
}

void AudioManager_PauseMusic() {
	Mix_PauseMusic(); //Pause the currently playing music
}

void AudioManager_ResumeMusic() {
	Mix_ResumeMusic(); //Resume the currently playing music
}

void AudioManager_PlayMusic() {
	if(bgMusic != NULL){
		Mix_PlayMusic(bgMusic, -1); //Play music infinitely
		Mix_VolumeMusic(MIX_MAX_VOLUME / 12); //Set the volume to 12th of the maximum volume)
	}
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