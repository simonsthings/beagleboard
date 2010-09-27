#!/bin/sh
#
echo "Hello, this script inserts your edited kernel files back into the kernel source tree and compiles and deploys it using bitbake."

export FULLKERNELDIR=/opt/oe/angstrom-dev/work/beagleboard-angstrom-linux-gnueabi/linux-omap-2.6.29-r89/git
export BITBAKEDEPLOYDIR=/opt/oe/angstrom-dev/deploy/glibc/images/beagleboard
export DRIVE_BOOT=/media/boot
export DRIVE_FS=/media/Angstrom

echo " The full Kernel sourcecode is located at $FULLKERNELDIR"
echo " The folder for the binary kernel images is $BITBAKEDEPLOYDIR"

# Insert:
#svn export ./arch/arm /opt/SVNtest/$FULLKERNELDIR/arch/arm/
svn export ./arch/arm $FULLKERNELDIR/arch/arm/ --force
#cp arch/arm/mach-omap2/* $FULLKERNELDIR/arch/arm/mach-omap2/ -R
#cp arch/arm/plat-omap/*  $FULLKERNELDIR/arch/arm/plat-omap/ -R

# Compile:
time bitbake -f -c compile linux-omap-2.6.29
echo "Ok, now deploying the freshly compiled image into a single binary file called uImage-something.bin."
# Deploy:
time bitbake -f -c deploy linux-omap-2.6.29
echo "The uImage has been created. Not trying to copy on to SD card..."

sudo ./copy-to-SDcard.sh