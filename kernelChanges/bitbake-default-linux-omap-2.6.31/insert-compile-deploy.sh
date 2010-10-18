#!/bin/sh
#
echo "Hello, this script inserts your edited kernel files back into the kernel source tree and compiles and deploys it using bitbake."

#export FULLKERNELDIR=/opt/oe/angstrom-dev/work/beagleboard-angstrom-linux-gnueabi/linux-omap-2.6.31-r89/git
export FULLKERNELDIR=/opt/angstrom-setup-scripts/build/tmp-angstrom_2008_1/work/beagleboard-angstrom-linux-gnueabi/linux-omap-2.6.31-r90/git
#export BITBAKEDEPLOYDIR=/opt/oe/angstrom-dev/deploy/glibc/images/beagleboard
export BITBAKEDEPLOYDIR=/opt/angstrom-setup-scripts/build/tmp-angstrom_2008_1/deploy/glibc/images/beagleboard
export DRIVE_BOOT=/media/boot
export DRIVE_FS=/media/Angstrom

echo " The full Kernel sourcecode is located at $FULLKERNELDIR"
echo " The folder for the binary kernel images is $BITBAKEDEPLOYDIR"
date 

# Insert back into Kernel:
svn export ./arch/arm $FULLKERNELDIR/arch/arm/ --force

# Compile:
echo "Starting to compile. The output is being fed to 'lastcompile.txt', so see there for any errors."
time bitbake -f -c compile linux-omap-2.6.31 > lastcompileoutput.txt
echo "Ok, now deploying the freshly compiled image into a single binary file called uImage-something.bin. Please wait..."
# Deploy:
time bitbake -f -c deploy linux-omap-2.6.31 > lastdeployoutput.txt
echo "The uImage has been created. Now trying to copy on to SD card..."
echo "The current time:"
date

sudo ./copy-to-SDcard.sh
