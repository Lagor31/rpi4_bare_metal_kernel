#!/bin/sh
openocd -f openocd_cfg/olimex.cfg -f openocd_cfg/rpi4.cfg 2>/dev/null  &
sleep 2 &
gdb-multiarch  --command=resume.cfg
echo "Cleaning up..."
