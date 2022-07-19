.global _wait_for_event  
.section .text
.type _wait_for_event, @function

_wait_for_event:
1:  wfe
	b 1b

.globl enable_irq      
.type enable_irq, @function
enable_irq:
    msr   daifclr, #2
    ret

.globl get_el
.type get_el, @function
get_el:
    mrs x0, CurrentEL
    and x0, x0, #12
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

.extern __stack_start   
.extern c_init_core 
.globl init_core
.type init_core, @function
init_core:   
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3

    ldr     x2, =__stack_start    // Get ourselves a fresh stack - location depends on CPU core asking
    lsl     x1, x1, #9           // Multiply core_number by 512
    add     x3, x2, x1           // Add to the address
    mov     sp, x3

    mov     x0, #0
    mov     x1, #0
    mov     x2, #0
    mov     x3, #0

    ldr     x4, =c_init_core
    blr     x4

1:
    wfe
    b       1b 