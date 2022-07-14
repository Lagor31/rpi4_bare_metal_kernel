#!/bin/sh
make clean all RPI=4
KERNEL=$(readlink -f build/kernel8.img)
echo $KERNEL
java -jar chainloader/LagorBoot.jar /dev/ttyUSB0  $KERNEL
