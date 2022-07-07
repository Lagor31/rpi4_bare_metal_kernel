#include "sysregs.h"

#if RPI==4
.set LOCAL_CONTROL, 0xff800000
.set LOCAL_PRESCALER, 0xff800008
#else
.set LOCAL_CONTROL, 0x40000000
.set LOCAL_PRESCALER, 0x40000008
#endif

.set OSC_FREQ, 54000000

//1MB for initial boot alloc, used to setup real kernel Heap allocator
.section ".boot_alloc"
.space 1 * 1024 * 1024, 0

.section ".heap"
.space 16 * 1024 * 1024, 0

.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:   
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f
    // We're not on the main core, so hang in an infinite wait loop
1:  wfe
    b       1b
2:  // We're on the main core!
_core_0:
 /* 
    ldr     x0, =LOCAL_CONTROL   // Sort out the timer
    str     wzr, [x0]
    mov     w1, 0x80000000
    str     w1, [x0, #(LOCAL_PRESCALER - LOCAL_CONTROL)]  */

/*      ldr     x0, =OSC_FREQ
    msr     cntfrq_el0, x0 */
    
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
    b       1b


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
    //eret

 1:    
    wfi
    b     1b 

.globl enable_irq      
.type enable_irq, @function
enable_irq:
    msr   daifclr, #2
    ret

.globl get_el
.type get_el, @function
get_el:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    ret

.globl disable_irq
.type disable_irq, @function
disable_irq:
    msr   daifset, #2
    ret


.globl get_core
.type get_core, @function
get_core:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF
    ret

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
