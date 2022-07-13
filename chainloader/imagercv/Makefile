#
# Copyright (C) 2020 bzt (bztsrc@github)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
#

CFLAGS = -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles
LDFLAGS = -nostdlib -nostartfiles

all: clean rpi4

ARM32CHAIN ?= aarch32-elf-
ARM64CHAIN ?= aarch64-linux-gnu-

rpi1: start32.S imgrecv.c
	$(ARM32CHAIN)gcc -DMMIO_BASE=0x20000000 -D__WORDSIZE=32 $(CFLAGS) -c imgrecv.c -o imgrecv.o
	@make --no-print-directory kernel7.img

rpi2: start32.S imgrecv.c
	$(ARM32CHAIN)gcc -DMMIO_BASE=0x3F000000 -D__WORDSIZE=32 $(CFLAGS) -c imgrecv.c -o imgrecv.o
	@make --no-print-directory kernel7.img

rpi3: start64.S imgrecv.c
	$(ARM64CHAIN)gcc -DMMIO_BASE=0x3F000000 -D__WORDSIZE=64 $(CFLAGS) -c imgrecv.c -o imgrecv.o
	@make --no-print-directory kernel8.img

rpi3_32: rpi2

rpi4: start64.S imgrecv.c
	$(ARM64CHAIN)gcc -DMMIO_BASE=0xFE000000 -D__WORDSIZE=64 $(CFLAGS) -c imgrecv.c -o imgrecv.o
	@make --no-print-directory kernel8.img

rpi4_32: start32.S imgrecv.c
	$(ARM32CHAIN)gcc -DMMIO_BASE=0xFE000000 -D__WORDSIZE=32 $(CFLAGS) -c imgrecv.c -o imgrecv.o
	@make --no-print-directory kernel7.img

kernel7.img: imgrecv.o
	$(ARM32CHAIN)gcc $(CFLAGS) -c start32.S -o start.o
	$(ARM32CHAIN)ld $(LDFLAGS) start.o imgrecv.o -T link32.ld -o kernel7.elf
	$(ARM32CHAIN)objcopy -O binary kernel7.elf kernel7.img

kernel8.img: imgrecv.o
	$(ARM64CHAIN)gcc $(CFLAGS) -c start64.S -o start.o
	$(ARM64CHAIN)ld $(LDFLAGS) start.o imgrecv.o -T link64.ld -o kernel8.elf
	$(ARM64CHAIN)objcopy -O binary kernel8.elf kernel8.img

clean:
	rm *.elf *.img *.o >/dev/null 2>/dev/null || true

