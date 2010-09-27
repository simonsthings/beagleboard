#!/bin/sh
# make sure these are the same as in insert-compile-deploy.sh !
export FULLKERNELDIR=/opt/oe/angstrom-dev/work/beagleboard-angstrom-linux-gnueabi/linux-omap-2.6.31-r89/git
export BITBAKEDEPLOYDIR=/opt/oe/angstrom-dev/deploy/glibc/images/beagleboard
export DRIVE_BOOT=/media/boot
export DRIVE_FS=/media/Angstrom

# Copy to SD card
echo "Now copying the created uImage and modules to the SD card!"
rm $DRIVE_FS/boot/uImage
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.31-r89-beagleboard.bin $DRIVE_FS/boot/uImage
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.31-r89-beagleboard.bin $DRIVE_FS/boot/
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.31-r89-beagleboard.bin $DRIVE_BOOT/uImage
cp -v $BITBAKEDEPLOYDIR/uImage-2.6.31-r89-beagleboard.bin $DRIVE_BOOT/

cp -v $BITBAKEDEPLOYDIR/modules-2.6.31-r89-beagleboard.tgz $DRIVE_FS/
cd $DRIVE_FS
echo "Unpacking freshly built modules..."
tar xzf modules-2.6.31-r89-beagleboard.tgz

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
