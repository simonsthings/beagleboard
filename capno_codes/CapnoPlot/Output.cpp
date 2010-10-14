#include <iostream>

#include "Output.h"

Output::Output(const char* title, unsigned int width, unsigned int height, unsigned char bpp): active(true) {
    this->surface   = NULL;

    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "SDL_INIT_EVERYTHING: " << SDL_GetError() << std::endl;
        return;
    }

    const   SDL_VideoInfo*      videoinfo;

    if((videoinfo = SDL_GetVideoInfo()) == 0) {
        std::cerr << "Video query failed: " << SDL_GetError() << std::endl;
        return;
    }

    int                         videoflags = SDL_HWPALETTE;

    if(videoinfo->hw_available != 0) {
        std::cout << "Using hardware surface." << std::endl;
        videoflags  |= SDL_HWSURFACE;
    } else {
        std::cout << "Using software surface." << std::endl;
        videoflags  |= SDL_SWSURFACE;
    }

    if(videoinfo->blit_hw != 0) {
        std::cout << "Hardware acceleration available." << std::endl;
    }

    if((this->surface = SDL_SetVideoMode(width, height, bpp, videoflags)) == NULL) {
        std::cerr << "Set Video Mode failed: " << SDL_GetError() << std::endl;
        return;
    }

    this->rectangle.x   = 0;
    this->rectangle.y   = 0;
    this->rectangle.w   = width;
    this->rectangle.h   = height;

    SDL_WM_SetCaption(title, NULL);
}

Output::~Output() {
    SDL_Quit();
}

unsigned char* Output::getBuffer() {
    return (unsigned char*)this->surface->pixels;
}

SDL_Surface* Output::getSurface() {
	return this->surface;
}

bool Output::flipBuffer() {
    SDL_UpdateRect(this->surface, 0, 0, this->rectangle.w, this->rectangle.h);

    return true;
}

bool Output::catchEvents() {
	SDL_Event							event;
	bool								run		= true;

	while(SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_ACTIVEEVENT:
			if(event.active.gain == 0) {
				this->active = false;
			} else {
				this->active = true;
			}
			break;
		case SDL_KEYDOWN:
			switch(event.key.keysym.sym) {
			case SDLK_ESCAPE:
				run	= false;
				break;
			default:
				break;
			}

			break;
		case SDL_QUIT:
			run = false;
			break;
		}
	}

	return run;
}
