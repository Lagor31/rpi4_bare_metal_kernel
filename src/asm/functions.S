#include "../include/Sysregs.h"
#include "../include/MMU.h"
.section ".text"


.global _test_set_lock
.type _test_set_lock, @function
_test_set_lock:
    mov	w2, #1
	ldaxr	w1, [x0]
	cbnz	w1, busy
	stxr	w1, w2, [x0]
    cbnz	w1, busy
    mov x0, 1
    b end
    busy:
    mov x0, 0

    end: 
	
    ret

.global _spin_lock
.type _spin_lock, @function
_spin_lock:
    mov	w2, #1
	sevl
l1:	wfe
l2:	ldaxr	w1, [x0]
	cbnz	w1, l1
	stxr	w1, w2, [x0]
    cbnz	w1, l2
	ret

.global _spin_unlock
.type _spin_unlock, @function
_spin_unlock:
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