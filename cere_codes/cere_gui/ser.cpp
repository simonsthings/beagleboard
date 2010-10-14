
/*

Linux Serial programming

Input: Serial port data from rSO2 cerebral Monitor.
Output: Display the result as a plot on the screen.

By: Abdallah Mohammedani

ver: 1.0

2009

*/

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include "SDL/SDL.h"



// Define Baude rate settings
#define BAUDRATE B9600

// Define the port
#define MODEMDEVICE "/dev/ttyUSB0"

using namespace std;

int prt_o, sig, vout;
struct termios oldtio, newtio;
char data[255];
int rso2;
int rso2_1;

void get_ser()
{

    /*  Open the device for reading and writing
        O_RDWR: Opens the port for reading and writing
        O_NOCTTY: The port never becomes the controlling terminal of the process.
        O_NDELAY: Use non-blocking I/O.
    */

    prt_o = open(MODEMDEVICE, O_RDWR | O_NOCTTY | O_NDELAY);


    // If there is no data print an error message
    if (prt_o < 0) {fprintf(stderr,"No USB Connection!!\n", SDL_GetError());exit(-1);}

   // save current serial port setting
    tcgetattr(prt_o,&oldtio);

    // Clear struct for the new port settings
    bzero(&newtio,sizeof(newtio));

    /* Control flags:
        Set the baudrate.
        CS8: Use charavter size of 8 bits.
        CREAD: Enable reciever else don't recieve any characters.
    */
    newtio.c_cflag = BAUDRATE | CS8 | CREAD;

    /* Input flags
        IGNPAR: Ignore bytes with parity errors.
        ICRNL: Translate an incoming carriage return to a new line character.
    */
    newtio.c_iflag = IGNPAR | ICRNL;

    /* Output flags */
    newtio.c_oflag = 0;

    /*  Local flags */
    newtio.c_lflag = CSTOPB;

    /* Intialize control characters */

    // one input is enough to return from read()
    newtio.c_cc[VMIN] = 1;

    // inter-character timer is off
    newtio.c_cc[VTIME] = 0;

    // Now clean the modem line and activate the settings for the port
    tcflush(prt_o,TCIFLUSH);
    tcsetattr(prt_o,TCSANOW,&newtio); // &var the pogram finds and grabsthe address reserved for var.

}

int Read_byte()

{
     sig = read(prt_o,data,512);

    char rs[3];
    char a[2] = {data[24]}; // data[24] is the 24th element in the array.
    char b[2] = {data[25]}; // data[24] is the 25th element in the array.

    strcpy(rs,a); // Pass the first char to the string rs.
    strcat(rs,b); // Put the second char to the same string.

    rso2_1 = atoi(rs); // Change from string to int.
    rso2 = rso2_1*5;
     ///////////////////////////////////


    return (sig);
}
