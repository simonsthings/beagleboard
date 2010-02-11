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
    int prt_o = open(mem,O_RDWR);
    volatile ulong *pinconf;
    if (prt_o<0) {cout<<"Can't open memory :( ..."; exit(-1);}

    // Map the register into memory
    pinconf = (ulong*) mmap(NULL, 0x10000,PROT_READ | PROT_WRITE,MAP_SHARED,prt_o,0x49050000);
    if(pinconf==MAP_FAILED) {cout<<"MAPPING FAILED :( ..."; close(prt_o);}
    //pinconf[0x6034/4] = 0xFFFFFFFF;

    for(;;)
    {
       
        pinconf[0x603C/4] == 0x600000;
        sleep(1);

        pinconf[0x603C/4] == 0xff9fffff;
        sleep(1);
    }
}

