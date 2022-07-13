Image Receiver for Raspberry Pi
===============================

This is the ARM version of the chain loader for Raspberry Pi computers. For rpi1 and rpi2, the image will be loaded at 0x8000, and
it can be in ELF32 or raw format, and will be executed in AArch32 mode. For rpi3 and rpi4, the load address is 0x80000, and ELF64
or raw format can be used, both executed in AArch64 mode. For AArch32 fallback, use rpi3_32 and rpi4_32. For the raw images, the
execution environment looks exactly the same as if they were loaded by start.elf. For ELFs the same is true, but their entry
point will be called not their first byte.

Compilation
-----------

Depending on your model, use one of:
```
make rpi1
make rpi2
make rpi3
make rpi3_32
make rpi4
make rpi4_32
```
Then copy "kernel*.img" to your SD card's first partition.

If your cross-compile toolchain is different from the default used in the Makefile, you can specify ARM32CHAIN or ARM64CHAIN
depending on the architecture. For example, to use `arm-none-eabi-gcc` to build rpi4_32:
```
make ARM32CHAIN=arm-none-eabi- rpi4_32
```

### Compiling on a Pi

To compile under Raspbian, you won't need a cross-compiler, the one that Raspbian is shipped with is perfectly fine. You can
simply specify an empty ARM32CHAIN variable to use the default compiler:
```
make ARM32CHAIN= rpi4_32
```
Note that Raspbian is a 32 bit system, so rpi1, rpi2, rpi3_32 and rpi4_32 will work, but in order to compile the AArch64 versions,
rpi3 and rpi4, you STILL NEED a cross-compiler.
