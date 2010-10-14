
/*

Linux Serial programming

Input: Serial port data from rSO2 cerebral Monitor.
Output: Display the result as a plot on the screen.

By: Abdallah Mohammedani

ver: 1.0

2009

*/




#include "ser.cpp"
#include "rtplot.cpp"
#include "alarm.cpp"
#include "font.cpp"


int main(int argc, char *args[])
{
    bool quit = false;

    get_ser();

    int x1,y1,x2,y2,ti;
    x1 = 51;
    y1 = 590;

    Read_byte();

    x2 = x1+1;
    y2 = y1-rso2;

    //Initialize
    if( initgraph() == false )
    {
        return 1;
    }

    Draw_coordi(); // Draw coordinates

    plot(x1,y1,x2,y2,165, 42, 42);

    while(quit==false)
    {
       	SDL_Event event;
       	while( SDL_PollEvent( &event ) )
       	{
       	    switch( event.type )
       	    {
       	        case SDL_QUIT:
       	        quit=true;
       	        break;
            }
	    }
        writing();

        Read_byte();

        x2 = x1+1; // Increment x2 value by 1 step.
        y1 = y2; // Define the new y1 position.

        // If there is a new value (not needed may be)
        if(y1!=y2)
        {
            y2 = 590 - (rso2); // Define the new y2 position.
        }
        x1=x2; // Define a new x position.
        ///////////////////////////////////
        // End of the time window
        if(x2 == 380)
        {

            plot_update();
            x1 = 52;
            y1= 590;
            x2 = x1+1;
            y2 = y1-(rso2);

        }
        //////////////////////////////////


        y2 = 590-(rso2); // Define y2 position.

        plot(x1,y1,x2,y2,165, 42, 42);

        ///////////////////////////////////
        //alarm
        if(rso2<=40)
        {

            alarm();

        }

        //////////////////////////////////


       sleep(1); // Update every 1 second.


    }

}
