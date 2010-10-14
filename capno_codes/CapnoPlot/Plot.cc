#include "Plot.h"

#include <iostream>
#include <cstring>

void Plot::drawline(short x, short y, short x2, short y2) {
	bool						ylonger		= false;
	short						incval, endval;
	short						shortlen	= y2 - y;
	short						longlen		= x2 - x;

	if(shortlen * shortlen > longlen * longlen) {
		short						swap		= shortlen;
		shortlen	= longlen;
		longlen		= swap;
		ylonger		= true;
	}

	endval	= longlen;

	if(longlen < 0) {
		incval		= -1;
		longlen		= -longlen;
	} else {
		incval		= 1;
	}

	float						decinc;

	if(longlen == 0) {
		decinc		= (float)shortlen;
	} else {
		decinc		= ((float)shortlen / (float)longlen);
	}

	float						j = 0.0f;

	if(ylonger) {
		for(int i = 0; i != endval; i += incval) {
			int						xx	= x + (int)j;
			int						yy	= y + i;

			int						id	= (this->width * yy + xx) * 3;

			this->pixels[id++]		= this->currb;
			this->pixels[id++]		= this->currg;
			this->pixels[id]		= this->currr;

			j	+= decinc;
		}
	} else {
		for(int i = 0; i != endval; i+= incval) {
			int						xx	= x + i;
			int						yy	= y + (int)j;

			int						id	= (this->width * yy + xx) * 3;

			this->pixels[id++]		= this->currb;
			this->pixels[id++]		= this->currg;
			this->pixels[id]		= this->currr;

			j	+= decinc;
		}
	}
}

Plot::Plot(SDL_Surface* surface, unsigned int length, unsigned int width, unsigned int height): surface(surface), pixels((unsigned char*)surface->pixels), length(length), width(width), height(height) {
	this->plotwidth		= width > length ? length : width;
	this->plotheight	= height / PLOT_COUNT;

	for(unsigned int i = 0; i < PLOT_COUNT; i++) {
		this->buffers[i]	= new short[length];

		this->r[i]		= 0xFF;
		this->g[i]		= 0x0;
		this->b[i]		= 0x0;

		this->min[i]	= 0;
		this->max[i]	= 50;
	}
}

Plot::~Plot() {
	for(unsigned int i = 0; i < PLOT_COUNT; i++) {
		delete[] this->buffers[i];
	}
}

void Plot::setPlotColor(unsigned int nr, unsigned char r, unsigned char g, unsigned char b) {
	this->r[nr]	= r;
	this->g[nr]	= g;
	this->b[nr]	= b;
}

void Plot::plot() {
	SDL_LockSurface(this->surface);

	memset(this->pixels, 0xFF, this->width * this->height * 3);

	for(unsigned int i = 0; i < PLOT_COUNT; i++) {
		short*				signal	= this->buffers[i];
		short				cmin = this->min[i], cmax = this->max[i];

		for(unsigned int x = 0; x < this->length; x++) {
			if(signal[x] > cmax) {
				cmax	= signal[x];
			}

			if(signal[x] < cmin) {
				cmin	= signal[x];
			}
		}

		this->min[i]	= cmin;
		this->max[i]	= cmax;

		if(cmin < -800) {
			this->min[i]	+= 10;
		}

		if(cmax > 800) {
			this->max[i]	-= 10;
		}

		this->currr		= this->r[i];
		this->currg		= this->g[i];
		this->currb		= this->b[i];

		float				diff	= cmax - cmin;
		float				scale	= (float)this->plotheight / diff;

		short				currheight	= (i + 1) * this->plotheight;

		this->drawline(0, currheight + (short)(scale * cmin), this->plotwidth, currheight + (short)(scale * cmin));

		short				y0	= currheight - (short)((float)(signal[0] - cmin) * scale) - 1;

		for(unsigned int x = 1; x < this->plotwidth; x++) {
			short				y1	= currheight - (short)((float)(signal[x] - cmin) * scale) - 1;

			this->drawline(x - 1, y0, x, y1);

			y0	= y1;
		}
	}

	SDL_UnlockSurface(this->surface);
}

void Plot::updatePlot(unsigned int nr, short* signal) {
	std::memcpy(this->buffers[nr], signal, this->length * sizeof(short));
}
