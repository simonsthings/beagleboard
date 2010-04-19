In order to compile this kernel module for use with your beagleboard 
(BB), you need to habe the omap-linux kernel source code somewhere on 
your machine, and it must be the correct version for the linux running
on your beagleboard.
Best is to use "bitbake -c compile linux-omap" or so. The exact command
is in Kunal's pinmux pdf. Adjust the paths in the .bb file and in the makefile.

Actually, just using "make" instead of "bitbake" for compiling the module
is enough as long as your kernel source is the same version as that 
running on the BB and your BB's uImage in the boot partition is binary
equal to that in the /boot directory in the ext3 partition.

