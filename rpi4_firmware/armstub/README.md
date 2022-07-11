# armstubsRPI4


make armstub8-gic.bin

Arm stubs for the raspberry PI 4 copied from here [https://github.com/raspberrypi/tools/tree/master/armstubs](https://github.com/raspberrypi/tools/tree/master/armstubs) and
edited to not drop the Exception Level of the processor and remain in EL3.

To make them work, copy the bin files to the root of your boot partition in the SDCard (FAT32 Partition) and add the line:

```
armstub=armstub8-gic.bin 
```
or 
```
armstub=armstub8.bin
```

depending if you want to use the gic controller or not.

Hope it helps

Cheers
Timanu

