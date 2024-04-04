#ifdef AUDIOMANAGER_H
#define AUDIOMANAGER_H

//Initiallize the audio system and SDL mixer
void AudioManager_Init();

//Load and play music from specified file path
void AudioManager_LoadAndPlayMusic(const char* file_path)

//Load music from specified file path [Didn't use anymore]
int AudioManager_LoadMusic(const char* file_path);

//Play the loaded music infinitely(Use -1 for looping infinitely) [Didn't use anymore]
void AudioManager_PlayMusic();

//Stop the music
void AudioManager_StopMusic();

//Load sound effect from specified file path
int AudioManager_LoadSoundEffect(const char* file_path);

//Play the loaded sound effect
void PlaySoundEffect();

//Frees the resource amd shuts down the audio system
void AudioManager_Cleanup();

#endif //AUDIOMANAGER_H

//Enumeration of sound effects
typedef enum {
	SOUND_SLASH,
	SOUND_CLICK,
	SOUND_GAMEOVER,
	SOUND_CONGRATS,
	SOUND_COUNT, //Number of sound effects
}SoundEffect;