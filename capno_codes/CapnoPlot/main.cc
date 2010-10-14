#include <iostream>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <cstring>


#include "unistd.h"
#include "signal.h"

#include "Output.h"
#include "Plot.h"
#include<cmath>
#include "SDL.h"


#define	DEFAULT_PLOT_WIDTH			640
#define	DEFAULT_PLOT_HEIGHT			480
#define DEFAULT_BPP					24

#define BAUDRATE					B9600
#define MODEMDEVICE					"/dev/ttyUSB0"

// Audio parameters
unsigned int Freq = 0;
unsigned int BufferSize = 0;
unsigned int out_AudioBufferSize = 0;

unsigned int freq1 = 500;
unsigned int fase1 = 0;

static	bool					run = true;

int s;


void sigcatch(int sig) {
    // Notify User.

    std::cout << "Caught Signal: ";

    switch(sig) {
    case SIGINT:
        std::cout << "SIGINT";
        break;
    case SIGQUIT:
        std::cout << "SIGQUIT";
        break;
    case SIGKILL:
        std::cout << "SIGKILL";
        break;
    case SIGTERM:
        std::cout << "SIGTERM";
        break;
    case SIGABRT:
        std::cout << "SIGABRT";
        break;
    case SIGTRAP:
        std::cout << "SIGTRAP";
        break;
    }

    std::cout << std::endl;

    // Clean Up.

	run	= false;

    // Say Goodbeye.

    std::cout << "Bye, Bye!" << std::endl;

	exit(-1);
}

int init_serial() {
	int fd,res;
	struct termios newtio;
	unsigned char buf[255];

	fd = open(MODEMDEVICE, O_RDWR | O_NONBLOCK);
	if (fd <0)
	{ perror(MODEMDEVICE); exit(-1); }

	bzero(&newtio, sizeof(newtio));

	newtio.c_cflag = BAUDRATE | CS8 | CREAD;
	newtio.c_iflag = 0;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */
	newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	newtio.c_cc[VERASE]   = 0;     /* del */
	newtio.c_cc[VKILL]    = 0;     /* @ */
	newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
	newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	newtio.c_cc[VSWTC]    = 0;     /* '\0' */
	newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */
	newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	newtio.c_cc[VEOL]     = 0;     /* '\0' */
	newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	newtio.c_cc[VEOL2]    = 0;     /* '\0' */

	tcflush(fd, TCIFLUSH);
	tcsetattr(fd,TCSANOW,&newtio);

	unsigned	char 		ididasv[4];

	ididasv[0]  = 0x85;
	ididasv[1]  = 0x1;
	ididasv[2]  = 0x4;
	ididasv[3]  = ididasv[1] ^ ididasv[2];

	unsigned	char		fms[5];

	fms[0]  = 0x85;
	fms[1]  = 0x2;
	fms[2]  = 53;
	fms[3]  = 0;
	fms[4]  = fms[1] ^ fms[2] ^ fms[3];

	write(fd,ididasv,4);
	write(fd, fms, 5);

	return fd;
}

void addValue(short* buffer, unsigned int length, unsigned char value) {
	for(int i = length - 1; i > 0; i--) {
		short				temp	= buffer[i];
		buffer[i]		= buffer[i - 1];
		buffer[i - 1]	= temp;
	}

	buffer[length - 1]	= value;
}

void getValue(int serial, short* buffer, unsigned int length) {
	unsigned	char			buf[256];
	int							res = 0;


	while((res = read(serial, buf, 255)) > 0) {
		if(buf[0] == 0x85 && buf[1] == 4 && buf[2] == 0 && buf[5] == 0) {
			addValue(buffer, length, buf[4]);

		}
	}
}

//Alarm setup

unsigned int SR;
unsigned int AudioBuffer;
unsigned int FR = 35;
unsigned int ti = 0;
double pi = 3.14;
double A = 100;

void Callback(void *unused, Uint8 *stream, int len)
{
    for (int i=0;i<len;i++)
    {
        stream[i] = int(A*sin((ti*2*pi*FR)/SR));
        ti++;
    }
}

int main(int argc, char* argv[]) {
	(void)signal(SIGINT, sigcatch);
	(void)signal(SIGQUIT, sigcatch);
	(void)signal(SIGKILL, sigcatch);
	(void)signal(SIGTERM, sigcatch);
	(void)signal(SIGABRT, sigcatch);
	(void)signal(SIGTRAP, sigcatch);

	unsigned	int		width			= DEFAULT_PLOT_WIDTH;
	unsigned	int		height 			= DEFAULT_PLOT_HEIGHT;
	unsigned	int		bpp				= DEFAULT_BPP;



	Output                          output("CapnoPlot", width, height, bpp);
	Plot							plot(output.getSurface(), width, width, height);

	plot.setPlotColor(0, 0xFF, 0, 0);

	short				temp[width];

	std::memset(temp, 0, width * sizeof(short));

	int					serial			= init_serial();

	while(run) {


////////////////////////////  ALARM START  ///////////////////////////////

		getValue(serial, temp, width);

		plot.updatePlot(0, temp);
		plot.plot();

		s=temp[4]; // The Capnometry value


		if ((s>10)&&(s<40))
		{
		    /* setup audio */
            SDL_AudioSpec *desired, *obtained;

            /* Allocate a space for desired SDL_AudioSpec */
            desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

            /* Allocate space for obtained SDL_AudioSpec */
            obtained = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

            /* Samplerate and audio-format */
            desired->freq = 1000;
            desired->format = AUDIO_S8;


            desired->samples = 10;

            /* callback function */
            desired->callback=Callback;
            desired->userdata=NULL;

            desired->channels = 1;

            /* Open the audio device and start playing sound! */
            if ( SDL_OpenAudio(desired, obtained) < 0 )
            {
                SDL_CloseAudio();
            }

            AudioBuffer = obtained->samples;
            SR = obtained->freq;

            SDL_PauseAudio(0);

		}

		if(s >= 40)
		{
			 /* setup audio */
            SDL_AudioSpec *desired, *obtained;

            /* Allocate a space for desired SDL_AudioSpec */
            desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

            /* Allocate space for obtained SDL_AudioSpec */
            obtained = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

            /* Samplerate and audio-format */
            desired->freq = 44100;
            desired->format = AUDIO_S8;


            desired->samples = 512;

            /* callback function */
            desired->callback=Callback;
            desired->userdata=NULL;

            desired->channels = 1;

            /* Open the audio device and start playing sound! */
            if ( SDL_OpenAudio(desired, obtained) < 0 )
            {
                SDL_CloseAudio();
            }

            AudioBuffer = 512;
            SR = 1024;

            SDL_PauseAudio(0);
		}


		else if(s <= 1)
		{

			sleep(4);
			if(s<=1)
			{
			     /* setup audio */
            SDL_AudioSpec *desired, *obtained;

            /* Allocate a space for desired SDL_AudioSpec */
            desired = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

            /* Allocate space for obtained SDL_AudioSpec */
            obtained = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));

            /* Samplerate and audio-format */
            desired->freq = 44100;
            desired->format = AUDIO_S8;


            desired->samples = 512;

            /* callback function */
            desired->callback=Callback;
            desired->userdata=NULL;

            desired->channels = 1;

            /* Open the audio device and start playing sound! */
            if ( SDL_OpenAudio(desired, obtained) < 0 )
            {
                SDL_CloseAudio();
            }

            AudioBuffer = 512;
            SR = 2000;

            SDL_PauseAudio(0);

		}
		}
		else
		{
			SDL_CloseAudio();
		}




///////////////////////  ALARM END  /////////////////////////////////
		if(!output.flipBuffer()) {
			std::cerr << "Output flipping Error." <<  std::endl;
			run = 0;
			break;
		}

		run	= output.catchEvents();
	}

	return 0;
}
