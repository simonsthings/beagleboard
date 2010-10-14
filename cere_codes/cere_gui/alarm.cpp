/*

Linux Serial programming

Alarm library.

By: Abdallah Mohammedani

ver: 1.0

2010

*/

#include "SDL/SDL_main.h"
#include <cmath>

unsigned int SR;
unsigned int AudioBuffer;

unsigned int FR = 50;
unsigned int ti = 0;
double pi = 3.14;
double A = 100;

void audio_(void *unused, Uint8 *stream, int len)
{

    for (int i=0;i<len;i++)
    {
        stream[i] = int(A*sin((ti*2*pi*FR)/SR));

        ti++;

    }
}

int alarm()
{

    /* setup audio */
    SDL_AudioSpec *wanted, *got;

    /* Allocate a desired SDL_AudioSpec */
    wanted = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

    /* Allocate space for the obtained SDL_AudioSpec */
    got = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

    /* choose a samplerate and audio-format */
    wanted->freq = 44100;
    wanted->format = AUDIO_S8;

    wanted->samples = 1000;

    /* callback function */
    wanted->callback=audio_;
    wanted->userdata=NULL;

    wanted->channels = 1;

    /* Open the audio device and start playing sound! */
    if ( SDL_OpenAudio(wanted, got) < 0 ) {
        SDL_CloseAudio();
    }

    AudioBuffer = 512;
    SR = 10000;

    SDL_PauseAudio(0);
}
