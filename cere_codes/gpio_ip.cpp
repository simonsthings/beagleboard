#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>

using namespace std;

#define mem "/dev/mem"

main(int argc, char **argv)
{
    int x;
    int prt_o = open(mem,O_RDWR | O_SYNC);
    if (prt_o<0) {cout<<"Can't open memory\n:( ..."; exit(-1);}

    // Pad configuration
     volatile ulong *pinconf;
     pinconf = (ulong*) mmap(NULL,0x10000,PROT_READ|PROT_WRITE,MAP_SHARED,prt_o,0x48000000);
     if(pinconf==MAP_FAILED) {cout<<"MAPPING FAILED \n:( ..."; close(prt_o); return 0;}

     pinconf[0x21BC/4] = 0x011C011C;
     pinconf[0x21C0/4] = 0x011C011C;

close(prt_o);

prt_o = open(mem,O_RDWR | O_SYNC);

    // Map the register into memory
	
	volatile ulong *gpio;

    gpio = (ulong*) mmap(NULL, 0x10000,PROT_READ | PROT_WRITE,MAP_SHARED,prt_o,0x49050000);
    if(gpio==MAP_FAILED) {cout<<"MAPPING FAILED\n:( ..."; close(prt_o); return 0;}

// Define pin_24 as input

gpio[0x8034/4] = 0x00000100;

    while(1)
    {
      x = gpio[0x8038/4];  // Get the data  
      if(x == 8388608) {cout<< "1" <<"\n";}  // output a value of one if the the pulse is high    
      if(x == 8388864) {cout<< "0" <<"\n";} // output a value of zero if the the pulse is low    
      usleep(500000); // Delay by 500 ms if the freq is 1 Hz
            
    }
}

