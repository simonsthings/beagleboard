
cd /media/boot
cp /media/boot/$1 /media/boot/uImage
sudo cp /media/boot/$1 /media/Angstrom/boot/uImage
echo Active uImage: /media/boot/$1 > /media/boot/activeImage.txt
