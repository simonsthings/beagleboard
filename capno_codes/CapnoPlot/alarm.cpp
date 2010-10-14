/*

Linux Serial programming

Alarm library.

By: Abdallah Mohammedani

ver: 1.0

2010

*/



unsigned int Freq = 0;
unsigned int BufferSize = 0;
unsigned int out_AudioBufferSize = 0;

unsigned int freq1 = 500;
unsigned int fase1 = 0;


void audio_(void *unused_data, Uint8 *stream, int len)
{

    unsigned int bytesPerPeriod = Freq / freq1;

    for (int i=0;i<len;i++) {
        int ch = int(100*sin(fase1*6.28/bytesPerPeriod));

        int outputValue = ch;          // Output channel


        stream[i] = outputValue;

        fase1++;
        fase1 %= bytesPerPeriod;

    }
}

int alarm()
{

    
    // setup audio
    SDL_AudioSpec *wanted, *got;

    // Allocate a desired SDL_AudioSpec
    wanted = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

    /* Allocate space for the obtained SDL_AudioSpec */
    got = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

    // sample rate and audio-format
    wanted->freq = 44100;
    wanted->format = AUDIO_S8;

    wanted->samples = 4096;

    // callback function 
    wanted->callback=audio_;
    wanted->userdata=NULL;

    wanted->channels = 1;

    // Open the audio device and start playing sound! 
    if ( SDL_OpenAudio(wanted, got) < 0 ) 
    {
        SDL_CloseAudio();
    }

    BufferSize = got->samples;
    Freq = got->freq;

    // if the format is 16 bit, two bytes are written for every sample 
    if (got->format==AUDIO_U16 || got->format==AUDIO_S16) 
    {
        out_AudioBufferSize = 2*BufferSize;
    } 
    else 
    {
        out_AudioBufferSize = BufferSize;
    }

	SDL_PauseAudio(0);
	SDL_Delay(1);         
}

