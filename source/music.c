#include <SDL2/SDL_mixer.h>
#include <mpg123.h>

#include "music.h"
#include "sdl.h"
#include "util.h"

#define sdl_default_bitrate 48000

static Mix_Music *music = NULL;
static mpg123_handle *handle = NULL;

static int min, sec;

void mp3Tag(char *music)
{
    mpg123_meta_free(handle);
    handle = mpg123_new(NULL, NULL);
    mpg123_param(handle, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.0);
    mpg123_open(handle, music);
    /*songSecTotal = */mpg123_framelength(handle) * mpg123_tpf(handle);//, min = songSecTotal / 60, sec = songSecTotal % 60;
    mpg123_id3v1 *v1 = NULL;
	mpg123_id3v2 *v2 = NULL;
    mpg123_seek(handle, 0, SEEK_CUR);
    //id3v2Found = false;

    /*if (sec < 9) secNeedsZero = true;
    else secNeedsZero = false;
    printf("\ntotal song length is %f\n", songSecTotal);
    if (secNeedsZero == true) printf("total song length in min/sec is %i:0%i\n", min, sec);
    else printf("total song length in min/sec is %i:%i\n", min, sec);

    songProgressBarIncrease = 525 / songSecTotal;
    printf("\n\nsong prog is %f\n", songProgressBarIncrease);*/
    
    if (mpg123_meta_check(handle) & MPG123_ID3 && mpg123_id3(handle, &v1, &v2) == MPG123_OK)
    {
		if (v1 != NULL)
        {
            printf("meta 1\n");
			//print_v1(&metadata, v1);
        }
		if (v2 != NULL) /*&& id3v2Enable == true)*/
        {
            printf("found idv2\n");
            for (size_t count = 0; count < v2->pictures; count++)
            {
                mpg123_picture *pic = &v2->picture[count];
                char *str = pic->mime_type.p;

                if ((pic->type == 3 ) || (pic->type == 0))
                {
                    if ((!strcasecmp(str, "image/jpg")) || (!strcasecmp(str, "image/jpeg")) || (!strcasecmp(str, "image/png")))
                    {
                        //id3v2Found = true;
                        imageLoadMem(&ID3_tag, pic->data, pic->size);
                        break;
                    }
                }
            }
        }
    }
}

int playMus(char **array, int file, int files_max)
{
    if (music != NULL)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
        //songProgressBar = 0;
    }
    //if (shuffleMode == true) file = randomizer(0, files_max);
    music = Mix_LoadMUS(array[file]);
    if (Mix_GetMusicType(music) == 6) mp3Tag(array[file]); 
    //else id3v2Found = false;
    Mix_PlayMusic(music, 0);
    //{ g_SongInSec = -1; g_SongInMin = 0; rewindValue = -1; }
    printf("\nNOW PLAYING... %s\n", array[file]);

    return file;
}

int skipnext(char **array, int file, int files_max)
{
    printf("\nSKIPNEXT\n");
    if (file + 1 <= files_max) file++;
    else file = 0;
    return playMus(array, file, files_max);
}

int skipback(char **array, int file, int files_max)
{
    printf("\nSKIPBACK\n");
    if (file - 1 >= 0) file--;
    else file = files_max;
    return playMus(array, file, files_max);
}

void musRewind(double value)
{
    if (value > 0) Mix_SetMusicPosition(value);
    else Mix_RewindMusic();
}

void musicInit()
{
    Mix_Init(MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC);
    Mix_OpenAudio(sdl_default_bitrate, AUDIO_S32LSB, 2, 1024);
    Mix_VolumeMusic(64); //TODO: volume slider
    mpg123_init();
}

void musicExit()
{
    Mix_HaltChannel(-1);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    Mix_Quit();
    mpg123_exit();
}