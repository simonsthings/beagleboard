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
cp arch/arm/mach-omap2/* $FULLKERNELDIR/arch/arm/mach-omap2/ -R
cp arch/arm/plat-omap/*  $FULLKERNELDIR/arch/arm/plat-omap/ -R

# Compile:
time bitbake -f -c compile linux-omap-2.6.29
echo "Ok, now deploying the freshly compiled image into a single binary file called uImage-something.bin."
# Deploy:
time bitbake -f -c deploy linux-omap-2.6.29
echo "The uImage has been created. Not trying to copy on to SD card..."

# Copy to SD card?
rm $DRIVE_FS/boot/uImage
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.29-r89-beagleboard.bin $DRIVE_FS/boot/uImage
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.29-r89-beagleboard.bin $DRIVE_FS/boot/
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.29-r89-beagleboard.bin $DRIVE_BOOT/uImage
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.29-r89-beagleboard.bin $DRIVE_BOOT/

cp -v $BITBAKEDEPLOYDIR/modules-2.6.29-r89-beagleboard.tgz $DRIVE_FS/
cd $DRIVE_FS
echo "Unpacking freshly built modules..."
tar xzf modules-2.6.29-r89-beagleboard.tgz

echo "Ok. Waiting for all data to be written to sd card (30 secs)!"
sleep 10
echo "20 more seconds.."
sleep 10
echo "10 more seconds.."
sleep 10
echo "Ok. Unmounting SD card now..."
umount $DRIVE_FS
umount $DRIVE_BOOT
echo "Finished. You should now be able to easily remove the SD card from this PC and boot the beagleboard with it."
