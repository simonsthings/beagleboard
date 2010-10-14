#include <string>
#include <time.h>
#include <string>
#include <sstream>
#include "SDL/SDL_ttf.h"
#include "SDL/SDL_image.h"


SDL_Surface *ti1, *ti2, *ti3, *ti4, *ti5 = NULL;
SDL_Surface *y_axis1, *y_axis2, *y_axis3, *y_axis4, *y_axis5 = NULL;
SDL_Surface *rso2_value1 = NULL;
SDL_Surface *time_axes = NULL;
SDL_Surface *rso2_axes = NULL;

//The font that's going to be used
TTF_Font *font1, *font2, *font3 = NULL;

SDL_Surface *bk = NULL;

//The color of the font
SDL_Color textColor = { 0, 0, 0 };
SDL_Color textColor1 = { 200, 0, 0 };


void apply_surface( int xx, int yy, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL )
{
    //Holds offsets
    SDL_Rect offset;

    //Get offsets
    offset.x = xx;
    offset.y = yy;

    //Blit
    SDL_BlitSurface( source, clip, destination, &offset );
}

SDL_Surface *load_image(string filename )
{
    //The image that's loaded
    SDL_Surface* loadedImage = NULL;

    //The optimized surface that will be used
    SDL_Surface* optimizedImage = NULL;

    //Load the image
    loadedImage = IMG_Load( filename.c_str() );

    //If the image loaded
    if( loadedImage != NULL )
    {
        //Create an optimized surface
        optimizedImage = SDL_DisplayFormat( loadedImage );

        //Free the old surface
        SDL_FreeSurface( loadedImage );

        //If the surface was optimized
        if( optimizedImage != NULL )
        {
            //Color key surface
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0, 0xFF, 0xFF ) );
        }
    }

    //Return the optimized surface
    return optimizedImage;
}

bool load_fonts()
{

    //Open the font
    font1 = TTF_OpenFont( "metro_df.ttf", 10);
    font2 = TTF_OpenFont( "metro_df.ttf", 15);
    font3 = TTF_OpenFont( "metro_df.ttf", 90);

    //If there was an error in loading the font
    if( font1== NULL )
    {
        return false;
    }

    else if( font2  == NULL )
    {
        return false;
    }
    else if( font3 == NULL )
    {
        return false;
    }


    //If everything loaded fine
    return true;
}

static void bkgr()

{
    SDL_Rect ho1, val1;

    // Horizontal
    ho1.x = 55;
    ho1.y = 600;
    ho1.w = 400;
    ho1.h = 35;

    val1.x = 450;
    val1.y = 25;
    val1.w = 200;
    val1.h = 100;

    SDL_FillRect(screen, &ho1, SDL_MapRGB( screen->format, 211, 211, 211 ));
    SDL_FillRect(screen, &val1, SDL_MapRGB( screen->format, 211, 211, 211 ));
}


int writing( )
{

             //Load the files
            if( load_fonts() == false )
            {
                return 1;
            }

            bkgr();

        stringstream rs_value1;
        rs_value1<< rso2_1;

        rso2_value1 = TTF_RenderText_Solid( font3, rs_value1.str().c_str(), textColor1 );
        apply_surface( 450, 25, rso2_value1, screen );

            struct tm * timeinfo;
            time_t rawtime;
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            mktime ( timeinfo );

            //The time as a string
            stringstream ttime1;
            stringstream ttime2;
            stringstream ttime3;
            stringstream ttime4;
            stringstream ttime5;

            //Convert the time to a string
            ttime1<< timeinfo->tm_hour<<":"<<timeinfo->tm_min+1;
            ttime2<< timeinfo->tm_hour<<":"<<timeinfo->tm_min+2;
            ttime3<< timeinfo->tm_hour<<":"<<timeinfo->tm_min+3;
            ttime4<< timeinfo->tm_hour<<":"<<timeinfo->tm_min+4;
            ttime5<< timeinfo->tm_hour<<":"<<timeinfo->tm_min+5;

            //Render the time surface

            ti1 = TTF_RenderText_Solid( font2, ttime1.str().c_str(), textColor );
            ti2 = TTF_RenderText_Solid( font2, ttime2.str().c_str(), textColor );
            ti3 = TTF_RenderText_Solid( font2, ttime3.str().c_str(), textColor );
            ti4 = TTF_RenderText_Solid( font2, ttime4.str().c_str(), textColor );
            ti5 = TTF_RenderText_Solid( font2, ttime5.str().c_str(), textColor );



    y_axis1 = TTF_RenderText_Solid( font1, "20", textColor );
    y_axis2 = TTF_RenderText_Solid( font1, "40", textColor );
    y_axis3 = TTF_RenderText_Solid( font1, "60", textColor );
    y_axis4 = TTF_RenderText_Solid( font1, "80", textColor );
    y_axis5 = TTF_RenderText_Solid( font1, "100", textColor );

    // Axes Definition
    rso2_axes = TTF_RenderText_Solid( font2, "rSO2 %", textColor );
    time_axes = TTF_RenderText_Solid( font2, "Time", textColor );
    //Apply surfaces
            apply_surface( 10, 10, rso2_axes, screen );
            apply_surface( 591, 600, time_axes, screen );

            apply_surface( 380, 600, ti5, screen );
            apply_surface( 310, 600, ti4, screen );
            apply_surface( 240, 600, ti3, screen );
            apply_surface( 170, 600, ti2, screen );
            apply_surface( 110, 600, ti1, screen );
            apply_surface( 20, 490, y_axis1, screen );
            apply_surface( 20, 390, y_axis2, screen );
            apply_surface( 20, 290, y_axis3, screen );
            apply_surface( 20, 190, y_axis4, screen );
            apply_surface( 20, 90, y_axis5, screen );

         //Update the screen
        if( SDL_Flip( screen ) == -1 )
        {
            return 1;
        }

    }
