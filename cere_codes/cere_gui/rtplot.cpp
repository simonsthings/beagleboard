/*

Linux Serial programming

Real Time plotting library.

By: Abdallah Mohammedani

ver: 1.0

2009

*/



#include <SDL/SDL_ttf.h>
#include <cmath>
#include <sstream>
#include <time.h>

#define SCREEN_WIDTH 670
#define SCREEN_HEIGHT 640

using  namespace std;


SDL_Surface *screen = NULL;   		//main drawing screen

/* Functions */

static void plot_update();
static void Draw_coordi();
bool initgraph();
void putpixel(int x, int y, Uint8 R, Uint8 G, Uint8 B);
static Uint32 GetColor( SDL_Surface *screen, Uint8 R, Uint8 G, Uint8 B );
void plot(int x1, int y1,int x2, int y2,  Uint8 R, Uint8 G, Uint8 B);
void update_plot(int x, int y,Uint8 R, Uint8 G, Uint8 B);
static void Lock( SDL_Surface *screen );
static void Unlock( SDL_Surface *screen );
void clean(void);


bool initgraph()
{

  int x= SCREEN_WIDTH, y= SCREEN_HEIGHT;

  if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
    {
      fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
      exit(-1);
    }
  atexit(SDL_Quit);

  // The caption
  SDL_WM_SetCaption("Regional Oxygen saturation (rSO2)", 0);

  screen = SDL_SetVideoMode(x, y, 8, SDL_SWSURFACE);

  if ( screen == NULL )
    {
    fprintf(stderr, "Unable to set video: %s\n", SDL_GetError());
    exit(-1);
    }

    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        fprintf(stderr, "Unable to set font: %s\n", SDL_GetError());
        exit(-1);
    }
    return true;
}

static Uint32 GetColor( SDL_Surface *screen, Uint8 R, Uint8 G, Uint8 B )
 {
     return SDL_MapRGB( screen->format, R, G, B );
 }

 static void Unlock( SDL_Surface *screen )
 {
     if (SDL_MUSTLOCK (screen))
     {
         SDL_UnlockSurface (screen);
     }
 }

void plot(int x1, int y1,int x2, int y2,  Uint8 R, Uint8 G, Uint8 B)
{
    int dx, dy, step_x,step_y, shift, points;

    dx = abs(x2 - x1);
    dy = abs(y1 - y2);

    while (y1!=y2)
    {
        putpixel(x1,y1,R,G, B);
        if(x1!=x2)
        {
            x1 += 1;
        }
        if(y2<y1)
        {
            y1 -= 1;
        }
        if(y2>y1)
        {
            y1 += 1;
        }
        SDL_Flip(screen);
    }

    while (x1!=x2)
    {
        putpixel(x1,y1,R, G, B);

        if (y2<y1)
        {
            y1 -= 1;
        }
        if (y2>y1)
        {
            y1 += 1;
        }
        if (y1=y2)
        {
            x1 += 1;
        }
        SDL_Flip(screen);
    }

    putpixel(x1,y1,R,G,B);
}

void putpixel(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{

    Uint32 bpp;

    Lock(screen);

    Uint8 *bufp;
    bpp = screen->format->BytesPerPixel;

    bufp = (Uint8 *) screen->pixels + y * screen->pitch + x*bpp;

    *bufp =  GetColor( screen, R, G, B );

    Unlock( screen );

   SDL_UpdateRect (screen, x, y, 1 , 1 );
}

 static void Lock( SDL_Surface *screen )
{
     if (SDL_MUSTLOCK (screen))
     {
         if (SDL_LockSurface (screen) < 0 )
         {
             fprintf (stderr,"Can't Lock screen: %s\n" , SDL_GetError ());
             return ;
         }
     }
}

static void Draw_coordi()

{
    SDL_Rect ho, v, pl, ticy1,ticy2,ticy3,ticy4,ticy5,ticx1,ticx2,ticx3,ticx4,ticx5;
    // Vertical
    v.x = 50;
    v.y = 50;
    v.w = 2;
    v.h = 550;

    // Horizontal
    ho.x = 40;
    ho.y = 591;
    ho.w = 550;
    ho.h = 2;

   // Plotting Area
    pl.x = 0;
    pl.y = 0;
    pl.w = 670;
    pl.h = 640;

    ticx1.x = 124;
    ticx1.y = 591;
    ticx1.w = 1;
    ticx1.h = 7;

    ticx2.x = 184;
    ticx2.y = 591;
    ticx2.w = 1;
    ticx2.h = 7;

    ticx3.x = 254;
    ticx3.y = 591;
    ticx3.w = 1;
    ticx3.h = 7;

    ticx4.x = 324;
    ticx4.y = 591;
    ticx4.w = 1;
    ticx4.h = 7;

    ticx5.x = 394;
    ticx5.y = 591;
    ticx5.w = 1;
    ticx5.h = 7;


    ticy1.x = 45;
    ticy1.y = 495;
    ticy1.w = 5;
    ticy1.h = 1;

    ticy2.x = 45;
    ticy2.y = 395;
    ticy2.w = 5;
    ticy2.h = 1;

    ticy3.x = 45;
    ticy3.y = 295;
    ticy3.w = 5;
    ticy3.h = 1;

    ticy4.x = 45;
    ticy4.y = 195;
    ticy4.w = 5;
    ticy4.h = 1;

    ticy5.x = 45;
    ticy5.y = 95;
    ticy5.w = 5;
    ticy5.h = 1;

    SDL_FillRect(screen, &pl, SDL_MapRGB( screen->format, 211, 211, 211));
    SDL_FillRect(screen, &v, SDL_MapRGB( screen->format, 0, 0, 0));
    SDL_FillRect(screen, &ho, SDL_MapRGB( screen->format, 0, 0, 0));
    SDL_FillRect(screen, &ticx1, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticx2, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticx3, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticx4, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticx5, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticy1, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticy2, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticy3, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticy4, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_FillRect(screen, &ticy5, SDL_MapRGB( screen->format, 0, 0, 0 ));
    SDL_UpdateRect (screen, 0, 0, 0 , 0 );
}

static void plot_update()

{
    SDL_Rect pl11;

    // Horizontal
    pl11.x = 53;
    pl11.y = 63;
    pl11.w = 366;
    pl11.h = 528;


    SDL_FillRect(screen, &pl11, SDL_MapRGB( screen->format, 211, 211, 211 ));
    //Update the screen
    SDL_Flip(screen);
}
