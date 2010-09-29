#!/bin/sh
#
echo "Hello, this script inserts your edited kernel files back into the kernel source tree and compiles and deploys it using bitbake."
date
export FULLKERNELDIR=/opt/oe/angstrom-dev/work/beagleboard-angstrom-linux-gnueabi/linux-omap-2.6.32-r89/git
export BITBAKEDEPLOYDIR=/opt/oe/angstrom-dev/deploy/glibc/images/beagleboard
export DRIVE_BOOT=/media/boot
export DRIVE_FS=/media/Angstrom

echo " The full Kernel sourcecode is located at $FULLKERNELDIR"
echo " The folder for the binary kernel images is $BITBAKEDEPLOYDIR"

# Insert back into Kernel:
svn export ./arch/arm $FULLKERNELDIR/arch/arm/ --force

# Compile:
time bitbake -f -c compile linux-omap-2.6.32
echo "Ok, now deploying the freshly compiled image into a single binary file called uImage-something.bin."
# Deploy:
time bitbake -f -c deploy linux-omap-2.6.32
echo "The uImage has been created. Not trying to copy on to SD card..."
date

sudo ./copy-to-SDcard.sh