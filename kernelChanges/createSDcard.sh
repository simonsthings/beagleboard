#! /bin/sh
# shell script by Simon Vogt

#export LC_ALL=C

export CARD_DEV=/dev/sdd
export DRIVE_BOOT=/media/boot
export DRIVE_FS=/media/Angstrom

export FS=/home/simon/backup/deleted_SD_cards/BOOT+ANGSTROM/ANGSTROM_partition/Angstrom-console-image-glibc-ipk-2009.X-stable-beagleboard.rootfs.tar.bz2
export BOOTSTUFF=/home/simon/backup/deleted_SD_cards/BOOT+ANGSTROM/BOOT_partition
export HOMESTUFF=/home/simon/backup/deleted_SD_cards/BOOT+ANGSTROM/ANGSTROM_partition/home/root
export U_IMAGE=/opt/oe/angstrom-dev/deploy/glibc/images/beagleboard/uImage-2.6.31-r89-beagleboard.bin
export MODULES=/opt/oe/angstrom-dev/deploy/glibc/images/beagleboard/modules-2.6.31-r89-beagleboard.tgz
export MKCARDSCRIPT=/home/simon/beagleboard/2.6.32-image/mkcard.sh

echo "Hi! This script will format your SD card $CARD_DEV!"
ls -l $CARD_DEV
mount | grep $CARD_DEV
echo "  Partitioning to be done by $MKCARDSCRIPT"
echo "It will also copy over all the needed files and unpack the file system."
echo "  $FS" 
echo "  $BOOTSTUFF/"
echo "  $HOMESTUFF/"
echo "  $U_IMAGE"
echo "  $MODULES"
echo "Did you start this script with sudo access? Are you sure you want to do this? (type \"y\")"
read CONT
if [ $CONT. == "y." ]; then
    echo You entered $CONT. OK! Continuing.;
else
    echo You entered $CONT. So exit.;
    exit 0;
fi


echo "Unmounting ${CARD_DEV}1 and ${CARD_DEV}2..."
mount | grep $CARD_DEV
umount ${CARD_DEV}1
umount ${CARD_DEV}2
echo "Creating new partitions... (waiting 3 secs)"
sleep 3
$MKCARDSCRIPT $CARD_DEV

echo
echo "Partitions were now probably created (assuming sudo access above)."
echo "Please disconnect usb card reader and then reconnect it!"
echo "Press enter when $DRIVE_BOOT and $DRIVE_FS have been mounted."
read
echo "Copying files..."
echo "   Angstrom FS:"
cp -v $FS $DRIVE_FS
cd $DRIVE_FS
tar xjf $FS
echo "   Boot Partition Stuff:"
cp -v $BOOTSTUFF/* $DRIVE_BOOT
echo "   Inserting uImage:"
cp -v $U_IMAGE $DRIVE_BOOT
cp -v $U_IMAGE $DRIVE_BOOT/uImage
cp -v $U_IMAGE $DRIVE_FS/boot
rm $DRIVE_FS/boot/uImage
cp -v $U_IMAGE $DRIVE_FS/boot/uImage
echo "   Home Directory Stuff:"
cp -v $HOMESTUFF/* $DRIVE_FS/home/root

echo 
echo "Ok. Waiting for all data to be written to sd card (20 secs)!"
sleep 10
echo 10 more..
sleep 10
echo "Unmounting the SD card now!"
umount $DRIVE_BOOT
umount $DRIVE_FS
echo "(Re-mount by plugging out and in.)"
echo "I think this was all. Insert into beagleboard and try booting!"

