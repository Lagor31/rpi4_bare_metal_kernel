/*
 * raspi/imgrecv.c
 * https://gitlab.com/bztsrc/imgrecv
 *
 * Copyright (C) 2020 bzt (bztsrc@github)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef MMIO_BASE
# error "Define MMIO_BASE for this SoC"
#endif

#define GPFSEL0         ((volatile unsigned int*)(MMIO_BASE+0x00200000))
#define GPFSEL1         ((volatile unsigned int*)(MMIO_BASE+0x00200004))
#define GPFSEL2         ((volatile unsigned int*)(MMIO_BASE+0x00200008))
#define GPFSEL3         ((volatile unsigned int*)(MMIO_BASE+0x0020000C))
#define GPFSEL4         ((volatile unsigned int*)(MMIO_BASE+0x00200010))
#define GPFSEL5         ((volatile unsigned int*)(MMIO_BASE+0x00200014))
#define GPSET0          ((volatile unsigned int*)(MMIO_BASE+0x0020001C))
#define GPSET1          ((volatile unsigned int*)(MMIO_BASE+0x00200020))
#define GPCLR0          ((volatile unsigned int*)(MMIO_BASE+0x00200028))
#define GPLEV0          ((volatile unsigned int*)(MMIO_BASE+0x00200034))
#define GPLEV1          ((volatile unsigned int*)(MMIO_BASE+0x00200038))
#define GPEDS0          ((volatile unsigned int*)(MMIO_BASE+0x00200040))
#define GPEDS1          ((volatile unsigned int*)(MMIO_BASE+0x00200044))
#define GPHEN0          ((volatile unsigned int*)(MMIO_BASE+0x00200064))
#define GPHEN1          ((volatile unsigned int*)(MMIO_BASE+0x00200068))
#define GPPUD           ((volatile unsigned int*)(MMIO_BASE+0x00200094))
#define GPPUDCLK0       ((volatile unsigned int*)(MMIO_BASE+0x00200098))
#define GPPUDCLK1       ((volatile unsigned int*)(MMIO_BASE+0x0020009C))

/* PL011 UART registers */
#define UART0_DR        ((volatile unsigned int*)(MMIO_BASE+0x00201000))
#define UART0_FR        ((volatile unsigned int*)(MMIO_BASE+0x00201018))
#define UART0_IBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201024))
#define UART0_FBRD      ((volatile unsigned int*)(MMIO_BASE+0x00201028))
#define UART0_LCRH      ((volatile unsigned int*)(MMIO_BASE+0x0020102C))
#define UART0_CR        ((volatile unsigned int*)(MMIO_BASE+0x00201030))
#define UART0_IMSC      ((volatile unsigned int*)(MMIO_BASE+0x00201038))
#define UART0_ICR       ((volatile unsigned int*)(MMIO_BASE+0x00201044))

/* AUX UART registers */
#define AUX_ENABLE      ((volatile unsigned int*)(MMIO_BASE+0x00215004))

/* mailbox registers */
#define MBOX_READ       ((volatile unsigned int*)(MMIO_BASE+0x0000B880+0x0))
#define MBOX_WRITE      ((volatile unsigned int*)(MMIO_BASE+0x0000B880+0x20))
#define MBOX_STATUS     ((volatile unsigned int*)(MMIO_BASE+0x0000B880+0x18))
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

#define MBOX_REQUEST    0
#define MBOX_TAG_SETCLKRATE     0x38002
#define MBOX_TAG_LAST           0

/* mailbox message buffer */
volatile unsigned int  __attribute__((aligned(16))) mbox[9] = {
    8*4, MBOX_REQUEST, MBOX_TAG_SETCLKRATE, 12, 8, 2, 4000000, 0 ,MBOX_TAG_LAST
};

/**
 * Send a character
 */
void uart_send(unsigned int c) {
    /* wait until we can send */
    do{asm volatile("nop");}while(*UART0_FR&0x20);
    /* write the character to the buffer */
    *UART0_DR=c;
}

/**
 * Receive a character
 */
char uart_getc() {
    /* wait until something is in the buffer */
    do{asm volatile("nop");}while(*UART0_FR&0x10);
    /* read it and return */
    return (char)(*UART0_DR);
}

/**
 * Delay cnt clockcycles
 */
void static inline delay(int cnt) { while(cnt--) { asm volatile("nop"); } }

/**
 * The main receiver function. Gets the load address, returns the entry point
 */
char *receiver(char *loadaddr)
{
    int size = 0;
    char *kernel = loadaddr;
    register unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | 8);

    /* initialize UART */
    *UART0_CR = 0;         /* turn off UART0 */
    *AUX_ENABLE = 0;       /* turn off UART1 */

    /* set up clock for consistent divisor values */
    do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_FULL);
    *MBOX_WRITE = r;
    do {
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
    } while(r != *MBOX_READ);

    /* map UART0 to GPIO pins */
    r=*GPFSEL1;
    r&=~((7<<12)|(7<<15)); /* gpio14, gpio15 */
    r|=(4<<12)|(4<<15);    /* alt0 */
    *GPFSEL1 = r;
    *GPPUD = 0;            /* enable pins 14 and 15 */
    delay(150);
    *GPPUDCLK0 = (1<<14)|(1<<15);
    delay(150);
    *GPPUDCLK0 = 0;        /* flush GPIO setup */

    *UART0_ICR = 0x7FF;    /* clear interrupts */
    *UART0_IBRD = 2;       /* 115200 baud */
    *UART0_FBRD = 0xB;
    *UART0_LCRH = 0x3<<5;  /* 8n1 */
    *UART0_CR = 0x301;     /* enable Tx, Rx, FIFO */

    while(1) {
        /* notify raspbootcom / USBImager to send the kernel */
        uart_send(3);
        uart_send(3);
        uart_send(3);

        /* read the kernel's size */
        size = uart_getc();
        size |= uart_getc()<<8;
        size |= uart_getc()<<16;
        size |= uart_getc()<<24;

        /* send negative or positive acknowledge */
        if(size < 32 || size >= 1024*1024*1024) {
            /* size error */
            uart_send('S');
            uart_send('E');
        } else {
            uart_send('O');
            uart_send('K');
            break;
        }
    }  

    
    /* read in the image */
    while(size--) *kernel++ = uart_getc();
    /* get entry point */
    return loadaddr[0] == 0x7f && loadaddr[1] == 'E' && loadaddr[2] == 'L' && loadaddr[3] == 'F' ?
#if __WORDSIZE == 64
        (char*)(*((unsigned long*)(loadaddr + 24)))
#else
        (char*)(*((unsigned int*)(loadaddr + 24)))
#endif
        : loadaddr;
}
