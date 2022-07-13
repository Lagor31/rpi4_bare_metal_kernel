#!/bin/sh
KERNEL=$(readlink -f build/kernel8.img)
echo $KERNEL
sudo java -jar chainloader/LagorBoot.jar /dev/ttyUSB0  $KERNEL
