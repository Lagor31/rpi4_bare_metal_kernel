#!/bin/sh
make clean && make RPI=4 && sudo mount /dev/$1 usb -o uid=1000,gid=1000 &&  cp build/kernel8.img usb && sync && sleep 3 && sudo umount usb
