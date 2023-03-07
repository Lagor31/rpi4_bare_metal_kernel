C/C++ Raspberry Pi 4B Kernel
- Developed on bare metal for Raspberry PI4 B with ARM v8 A 
- Premptive Multitasking
- Interrupts (GIC400)
- Multicore
- Virtual memroy MMU
- Synchronization (ARM Spinlock)
- Uart
- Mailbox
- Framebuffer

Debugger on live hardware with OpenOCD + JTAGS + Olimex Tiny H (https://www.olimex.com/Products/ARM/JTAG/ARM-USB-TINY-H/)
Install openocd from sources with     ./configure --enable-ftdi;                               

I/O Through MiniUART/USB with minicom
sudo minicom -D /dev/ttyUSB0 --baudrate 11520

[Work in progres...]

