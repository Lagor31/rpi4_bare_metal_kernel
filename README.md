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
### Debuggint with minicom and GDB

![osdev_debug](https://user-images.githubusercontent.com/20226839/225079906-9a0c62b9-1d42-46eb-b9c7-4711667ccf51.png)

### Live Demo - The color of every circle gets determined by which core is running the relative rendering task
[video_2023-03-14_17-59-56.webm](https://user-images.githubusercontent.com/20226839/225082004-a62e8bbf-50ec-4bd3-9640-4f63e673bd0b.webm)
### TODO:
- Unix Pipes
- System calls
- Shell
- Users, Groups & Permissions
- EMMC Driver
- Custom filesystem
- ELF Loader
- Slab Allocator
- On Disk Paging
- Usermode Tasks
- Networking (Ethernet, ARP, UDP, DHCP)
- USB Driver
- (...)

[Work in progres...]

