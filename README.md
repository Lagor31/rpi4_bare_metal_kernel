## C/C++ Raspberry Pi 4B Kernel
### Features
- Developed on bare metal for Raspberry PI4 B with Cortex A72 (ARM v8.A) 
- Premptive Multitasking
- Interrupts (GIC400)
- Multicore
- Virtual memory & MMU
- Buddy Allocator
- Synchronization (ARM Spinlock)
- UART RX/TX
- Videocore VI Mailbox Interface
- Framebuffer (HDMI output)

### Debug & Testing
Debugged on live hardware with OpenOCD + JTAGS + Olimex Tiny H (https://www.olimex.com/Products/ARM/JTAG/ARM-USB-TINY-H/)  
  
Install **openocd** from sources with  
```
./configure --enable-ftdi
```

I/O Through MiniUART to USB with **minicom**  
```
sudo minicom -D /dev/ttyUSB0 --baudrate 11520
```

### TODO:
- Unix Pipes
- System calls
- EMMC Driver
- Custom filesystem
- ELF Loader
- Slab Allocator
- On Disk Paging
- Usermode Tasks
- Networking
- USB Driver
- (...)

[Work in progres...]
