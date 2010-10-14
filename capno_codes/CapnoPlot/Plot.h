#ifndef PLOT_H
#define PLOT_H

#include "SDL/SDL.h"

#define	PLOT_COUNT	1

class Plot {
private:
	SDL_Surface*				surface;

	short*						buffers[PLOT_COUNT];
	unsigned 	char			r[PLOT_COUNT], g[PLOT_COUNT], b[PLOT_COUNT], currr, currg, currb;

	short						min[PLOT_COUNT], max[PLOT_COUNT];

	unsigned	char*			pixels;

	unsigned	int				length, width, height, plotheight, plotwidth;

	void drawline(short x, short y, short x2, short y2);

public:
	Plot(SDL_Surface* surface, unsigned int length, unsigned int width, unsigned int height);
	~Plot();

	void setPlotColor(unsigned int nr, unsigned char r, unsigned char g, unsigned char b);

	void plot();
	void updatePlot(unsigned int nr, short* signal);
};

#endif // PLOT_H
