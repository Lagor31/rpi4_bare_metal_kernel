C++FILES = $(wildcard src/*.cpp src/**/*.cpp )
ASMFILES = $(wildcard **/*.S)
OBJ = ${C++FILES:.cpp=.o}


GCCFLAGS = -Wall -O2 -ffreestanding -fno-exceptions -fno-rtti
G++ = aarch64-linux-gnu-g++
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
RPI?=3

all: clean kernel8.img

boot.o: src/boot/boot.S
	$(G++) $(GCCFLAGS) -c src/boot/boot.S -o src/boot/boot.o

%.o: %.cpp
	$(G++) $(GCCFLAGS) -DRPI=$(RPI) -c $< -o $@

kernel8.img: boot.o ${OBJ}
	mkdir -p build
	$(LD) -nostdlib src/boot/boot.o $(OBJ) -T linker.ld -o build/kernel8.elf
	$(OBJCOPY) -O binary build/kernel8.elf build/kernel8.img

qemu: clean kernel8.img 
	qemu-system-aarch64 -M raspi3 -serial null -serial stdio -kernel kernel8.img

clean:
	/bin/rm  -f kernel8.elf src/*.o src/**/*.o build > /dev/null 2> /dev/null || true