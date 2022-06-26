C++FILES = $(wildcard src/*.cpp src/**/*.cpp )
HEADERS = $(wildcard src/*.h src/**/*.h )
ASMFILES = $(wildcard **/*.S)
OBJ = ${C++FILES:.cpp=.o}


GCCFLAGS = -Wall -O2 -ffreestanding -fno-exceptions -fno-rtti
G++ = aarch64-linux-gnu-g++
LD = aarch64-linux-gnu-ld
OBJCOPY = aarch64-linux-gnu-objcopy
GDB = gdb-multiarch
RPI?=3

all: clean kernel8.img

boot.o: src/boot/boot.S
	$(G++) $(GCCFLAGS) -c src/boot/boot.S -o src/boot/boot.o

%.o: %.cpp ${HEADERS}
	$(G++) $(GCCFLAGS) -DRPI=$(RPI) -g -O0 -c $< -o $@

kernel8.img: boot.o ${OBJ}
	mkdir -p build
	$(LD) -nostdlib src/boot/boot.o $(OBJ) -n -T linker.ld -o build/kernel8.elf
	$(OBJCOPY) -O binary build/kernel8.elf build/kernel8.img

debug: kernel8.img 
	qemu-system-aarch64 -S -d cpu_reset -s -M raspi3 -serial null -serial stdio -kernel build/kernel8.elf &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file build/kernel8.elf"

run: kernel8.img 
	qemu-system-aarch64 -d cpu_reset -s -M raspi3 -serial null -serial stdio -kernel build/kernel8.elf 

clean:
	/bin/rm  -f src/*.o src/**/*.o build > /dev/null 2> /dev/null || true