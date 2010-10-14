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
    int prt_o = open(mem,O_RDWR | O_SYNC);
    if (prt_o<0) {cout<<"Can't open memory\n:( ..."; exit(-1);}

    // Pad configuration
     volatile ulong *pinconf;
     pinconf = (ulong*) mmap(NULL,0x10000,PROT_READ|PROT_WRITE,MAP_SHARED,prt_o,0x48000000);
     if(pinconf==MAP_FAILED) {cout<<"MAPPING FAILED \n:( ..."; close(prt_o); return 0;}

     pinconf[0x21BC/4] = 0x001C001C;
     pinconf[0x21C0/4] = 0x001C001C;

close(prt_o);

prt_o = open(mem,O_RDWR | O_SYNC);

    // Map the register into memory
	
	volatile ulong *gpio;

    gpio = (ulong*) mmap(NULL, 0x10000,PROT_READ | PROT_WRITE,MAP_SHARED,prt_o,0x49050000);
    if(gpio==MAP_FAILED) {cout<<"MAPPING FAILED\n:( ..."; close(prt_o); return 0;}

    while(1)
    {
        gpio[0x803C/4] = 0x800000;
        usleep(10000);
        gpio[0x803C/4] = 0x0;
        usleep(30000);       
        gpio[0x803C/4] = 0x100;
        usleep(10000);
        gpio[0x803C/4] = 0x0;
        usleep(30000);       
    }
}

