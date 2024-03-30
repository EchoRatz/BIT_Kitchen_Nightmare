#ifdef AUDIOMANAGER_H
#define AUDIOMANAGER_H

//Initiallize the audio system and SDL mixer
void AudioManager_Init();

//Load music from specified file path
int AudioManager_LoadMusic(const char* file_path);
//Return 1 on sucess, 0 on failure

//Play the loaded music infinitely(Use -1 for looping infinitely)
void AudioManager_PlayMusic();

//Stop the music
void AudioManager_StopMusic();

//Frees the resource amd shuts down the audio system
void AudioManager_Cleanup();

#endif //AUDIOMANAGER_H