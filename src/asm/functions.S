#include "../include/sysregs.h"
#include "../include/Mmu.h"

.section ".text"


.globl core_switch_to
.type core_switch_to, @function
core_switch_to:
	
    mov x9, sp
    /* save context into prev->core_context */
    stp x19, x20, [x0], #16
    stp x21, x22, [x0], #16
    stp x23, x24, [x0], #16
    stp x25, x26, [x0], #16
    stp x27, x28, [x0], #16
    stp x29, x9,  [x0], #16
    str x30, [x0]
    /* restore context from next->core_context */
    ldp x19, x20, [x1], #16
    ldp x21, x22, [x1], #16
    ldp x23, x24, [x1], #16
    ldp x25, x26, [x1], #16
    ldp x27, x28, [x1], #16
    ldp x29, x9,  [x1], #16
    ldr x30, [x1]
    mov sp, x9
_stop_here:
    ret
    
    
    
    
    
    
    
    
    
    /* 
    // Store all general purpose registers on the stack.
	stp	x0,  x1,  [x0, #16 * 0]
	stp	x2,  x3,  [x0, #16 * 1]
	stp	x4,  x5,  [x0, #16 * 2]
	stp	x6,  x7,  [x0, #16 * 3]
	stp	x8,  x9,  [x0, #16 * 4]
	stp	x10, x11, [x0, #16 * 5]
	stp	x12, x13, [x0, #16 * 6]
	stp	x14, x15, [x0, #16 * 7]
	stp	x16, x17, [x0, #16 * 8]
	stp	x18, x19, [x0, #16 * 9]
	stp	x20, x21, [x0, #16 * 10]
	stp	x22, x23, [x0, #16 * 11]
	stp	x24, x25, [x0, #16 * 12]
	stp	x26, x27, [x0, #16 * 13]
	stp	x28, x29, [x0, #16 * 14]

	// Add the exception link register (ELR_EL1), saved program status (SPSR_EL1) and exception
	// syndrome register (ESR_EL1).
	//mrs	x1,  ELR_EL1
	mrs	x2,  SPSR_EL1
	mrs	x3,  ESR_EL1
	stp	lr,  lr,  [x0, #16 * 15]
	stp	x2,  x3,  [x0, #16 * 16] // esr = lr
    mov x3, sp
    mov x4, #0
    stp x3,x4,[x0, #16 * 17]
	
    
    ldr	w19,      [x1, #16 * 16]
	ldp	lr,  x20, [x1, #16 * 15]

	msr	SPSR_EL1, x19
	msr	ELR_EL1,  x20

	ldp	x2,  x3,  [x1, #16 * 1]
	ldp	x4,  x5,  [x1, #16 * 2]
	ldp	x6,  x7,  [x1, #16 * 3]
	ldp	x8,  x9,  [x1, #16 * 4]
	ldp	x10, x11, [x1, #16 * 5]
	ldp	x12, x13, [x1, #16 * 6]
	ldp	x14, x15, [x1, #16 * 7]
	ldp	x16, x17, [x1, #16 * 8]
	ldp	x18, x19, [x1, #16 * 9]
	ldp	x20, x21, [x1, #16 * 10]
	ldp	x22, x23, [x1, #16 * 11]
	ldp	x24, x25, [x1, #16 * 12]
	ldp	x26, x27, [x1, #16 * 13]
    ldp x28, x29, [x1, #16 * 17]
    msr SP_EL0, x28
    mov sp, x28
	ldp	x28, x29, [x1, #16 * 14]

	ldp	x0,  x1,  [x1, #16 * 0]
    //msr     SPSel, #0
_stop_here:
	ret
 */

.global spin_lock
.type spin_lock, @function
spin_lock:
    mov	w2, #1
	sevl
l1:	wfe
l2:	ldaxr	w1, [x0]
	cbnz	w1, l1
	stxr	w1, w2, [x0]
    cbnz	w1, l2
	ret

.global spin_unlock
.type spin_unlock, @function
spin_unlock:
    stlr	wzr, [x0]
	ret


.global _hang_forever  
.type _hang_forever, @function
_hang_forever:
1:  wfe
	b 1b

.global _wait_for_interrupt  
.type _wait_for_interrupt, @function
_wait_for_interrupt:
1:  wfi
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

.globl get_sp
.type get_sp, @function
get_sp:
    mov x0, sp
    ret


.globl get_esr_el1
.type get_esr_el1, @function
get_esr_el1:
    mrs x0, ESR_EL1
    ret 

.globl get_elr_el1
.type get_elr_el1, @function
get_elr_el1:
    mrs x0, ELR_EL1
    ret 


.globl get_far_el1
.type get_far_el1, @function
get_far_el1:
    mrs x0, FAR_EL1
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


.section ".boot"
.extern __stack_start   
.extern initSecondaryCore 
.globl init_core
.type init_core, @function
init_core:   
   
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

    mrs     x1, mpidr_el1
    and     x1, x1, #3

    ldr     x2, =__cpu0_stack    // Get ourselves a fresh stack - location depends on CPU core asking
    lsl     x1, x1, #12           // Multiply core_number by 4096
    add     x3, x2, x1           // Add to the address
    mov     sp, x3
    msr     sp_el1, x3
    // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret   
5:
    msr     SPSel, #1

    ldr x0, =id_pg_dir
    msr ttbr0_el1, x0
    ldr x0, =high_pg_dir
    msr ttbr1_el1, x0

    ldr x0, =TCR_EL1_VAL
    msr TCR_EL1, x0

    ldr x0, =MAIR_EL1_VAL
    msr MAIR_EL1, x0
    
    //Init MMU Here with &kernel_main = virt addr of kernel

    mov    x0, #5
    dsb ish
    isb

    msr    sctlr_el1, x0
    isb
    dsb ish

    mov     x0, #0
    mov     x1, #0
    mov     x2, #0
    mov     x3, #0

    ldr     x4, =initSecondaryCore
    br     x4

1:
    wfe
    b       1b 