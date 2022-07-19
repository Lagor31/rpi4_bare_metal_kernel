#include "sysregs.h"

.set LOCAL_CONTROL, 0xff800000
.set LOCAL_PRESCALER, 0xff800008


.set OSC_FREQ, 54000000

//1MB for initial boot alloc, used to setup real kernel Heap allocator
.section ".boot_alloc"
.space 1 * 1024 * 1024, 0

.section ".heap"
.space 16 * 1024 * 1024, 0

.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:   
    //w0 = dtb addr 32 bit
    //In our setup, only core0@EL3 gets here

    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3 
    sub     x1, x1, #0 // We want CORE0
    cbz     x1, _core_0 
    // We're not on the main core, so hang in an infinite wait loop
    adr     x5, spin_cpu0
1:  wfe
_checking_addr:
    ldr     x4, [x5, x1, lsl #3]
    cmp	x4, 0
	beq	1b
    //cbz     x4, 1b

_core_active:
    ldr     x2, =__stack_start    // Get ourselves a fresh stack - location depends on CPU core asking
    lsl     x1, x1, #9           // Multiply core_number by 512
    add     x3, x2, x1           // Add to the address
    mov     sp, x3

    mov     x0, #0
    mov     x1, #0
    mov     x2, #0
    mov     x3, #0    
    blr     x4 

    b       1b  
2:  // We're on the main core!
_core_0:
 /* 
    ldr     x0, =LOCAL_CONTROL   // Sort out the timer
    str     wzr, [x0]
    mov     w1, 0x80000000
    str     w1, [x0, #(LOCAL_PRESCALER - LOCAL_CONTROL)]  */

    ldr     x0, =OSC_FREQ
    msr     cntfrq_el0, x0 
    
    //TODO: PRoblematic
    /*  
    mov     x0, #0x33ff
    msr     cptr_el3, x0 	 // Disable coprocessor traps to EL3
    mov     x0, #3 << 20
    msr     cpacr_el1, x0	 // Enable FP/SIMD at EL1 
    */

    // Set stack to start below our code
    ldr     x1, =_start
 
    mrs     x0, CurrentEL
    and     x0, x0, #12 // clear reserved bits

    cmp     x0, #12 //EL == 3
    bne     5f

    mov     x2, #0x5b1
    msr     scr_el3, x2
    mov     x2, #0x3c9
    msr     spsr_el3, x2
    adr     x2, 5f
    msr     elr_el3, x2
    eret 

5:  cmp     x0, #4 //EL==1
    beq     5f

    msr     sp_el1, x1
    // enable CNTP for EL1
    mrs     x0, cnthctl_el2
    orr     x0, x0, #3
    msr     cnthctl_el2, x0
    msr     cntvoff_el2, xzr
    // enable AArch64 in EL1
    mov     x0, #(1 << 31)      // AArch64
    orr     x0, x0, #(1 << 1)   // SWIO hardwired on Pi3
    msr     hcr_el2, x0
    mrs     x0, hcr_el2

    ldr x0, =vector
    msr vbar_el1, x0 

  // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret   

5:  mov     sp, x1

    // Clean the BSS section
    ldr     x1, =__bss_start     // Start address
    ldr     w2, =__bss_size      // Size of the section
3:  cbz     w2, 4f               // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b               // Loop if non-zero

    // Jump to our main() routine in C (make sure it doesn't return)
4:  bl      kernel_main
    // In case it does return, halt the master core too
    b       1f


.global panic
hang:
    stp   x0,  x1,  [sp, #-16]!
    stp   x2,  x3,  [sp, #-16]!
    stp   x4,  x5,  [sp, #-16]!
    stp   x6,  x7,  [sp, #-16]!
    stp   x8,  x9,  [sp, #-16]!
    stp   x10, x11, [sp, #-16]!
    stp   x12, x13, [sp, #-16]!
    stp   x14, x15, [sp, #-16]!
    stp   x16, x17, [sp, #-16]!
    stp   x18, x19, [sp, #-16]!

    bl panic

    ldp   x18, x19, [sp], #16
    ldp   x16, x17, [sp], #16
    ldp   x14, x15, [sp], #16
    ldp   x12, x13, [sp], #16
    ldp   x10, x11, [sp], #16
    ldp   x8,  x9,  [sp], #16
    ldp   x6,  x7,  [sp], #16
    ldp   x4,  x5,  [sp], #16
    ldp   x2,  x3,  [sp], #16
    ldp   x0,  x1,  [sp], #16

 1:    
    wfi
    b     1b 


irq:
    stp   x0,  x1,  [sp, #-16]!
    stp   x2,  x3,  [sp, #-16]!
    stp   x4,  x5,  [sp, #-16]!
    stp   x6,  x7,  [sp, #-16]!
    stp   x8,  x9,  [sp, #-16]!
    stp   x10, x11, [sp, #-16]!
    stp   x12, x13, [sp, #-16]!
    stp   x14, x15, [sp, #-16]!
    stp   x16, x17, [sp, #-16]!
    stp   x18, x19, [sp, #-16]!

    bl    irq_h

    ldp   x18, x19, [sp], #16
    ldp   x16, x17, [sp], #16
    ldp   x14, x15, [sp], #16
    ldp   x12, x13, [sp], #16
    ldp   x10, x11, [sp], #16
    ldp   x8,  x9,  [sp], #16
    ldp   x6,  x7,  [sp], #16
    ldp   x4,  x5,  [sp], #16
    ldp   x2,  x3,  [sp], #16
    ldp   x0,  x1,  [sp], #16
    eret

.balign 4096
vector:
.balign 128
    b hang
.balign 128
    b irq
.balign 128
    b hang
.balign 128
    b hang
.balign 128
    b hang
.balign 128
    b irq
.balign 128
    b hang
.balign 128
    b hang
.balign 128
    b hang
.balign 128
    b irq
.balign 128
    b hang
.balign 128
    b hang
.balign 128
    b hang
.balign 128
    b irq
.balign 128
    b hang
.balign 128
    b hang

 .balign 4096
.ltorg
.globl spin_cpu0
spin_cpu0:
        .quad 0

.globl spin_cpu1
spin_cpu1:
        .quad 0

.globl spin_cpu2
spin_cpu2:
        .quad 0

.globl spin_cpu3
spin_cpu3:
        .quad 0 

 