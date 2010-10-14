#ifndef OUTPUT_H
#define OUTPUT_H

#include "SDL.h"

class Output {
private:
    SDL_Surface*                surface;
    SDL_Rect                    rectangle;

    bool						active;

public:
    Output(const char* title, unsigned int width, unsigned int height, unsigned char bpp);
    ~Output();

    unsigned char* getBuffer();
    SDL_Surface* getSurface();
    bool flipBuffer();
    bool catchEvents();
};

#endif
