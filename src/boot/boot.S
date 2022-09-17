#include "../include/sysregs.h"
#include "../include/Mmu.h"


//1MB for initial boot alloc, used to setup real kernel Heap allocator
.section ".boot_alloc"
.space 1 * 1024 * 1024, 0

.section ".heap"
.space 16 * 1024 * 1024, 0

.section ".boot", #alloc, #execinstr // Make sure the linker puts this at the start of the kernel image


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
1:  wfe
    b 1b

2:  // We're on the main core!
_core_0:

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

_setup_core0_stack:
    //Need to use Virtual addresses
    ldr     x1, =__cpu0_stack
    msr     sp_el1, x1
    mov     sp, x1

  // change execution level to EL1
    mov     x2, #0x3c4
    msr     spsr_el2, x2
    adr     x2, 5f
    msr     elr_el2, x2
    eret   
5: 

    msr     SPSel, #1
    
    bl map_identity
    bl map_high

_load_ttbr:
    ldr x0, =id_pg_dir
    msr ttbr0_el1, x0
    ldr x0, =high_pg_dir
    msr ttbr1_el1, x0
    // In case it does return, halt the master core too

    /* setup TCR_EL1 */
    ldr x0, =TCR_EL1_VAL
    msr TCR_EL1, x0

    /* setup MAIR_EL1 */
    ldr x0, =MAIR_EL1_VAL
    msr MAIR_EL1, x0
    
_start_mmu:
    //Init MMU Here with &kernel_main = virt addr of kernel
    mov    x0, #5 //ENABLE MMU, CACHE ENABLE
    msr    sctlr_el1, x0
    isb

    /* ldr     x1, =__cpu0_stack
    mov     sp, x1 */

    // Clean the BSS section
    ldr     x1, =__bss_start     // Start address
    ldr     w2, =__bss_size      // Size of the section
3:  cbz     w2, 4f               // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #1
    cbnz    w2, 3b               // Loop if non-zero  

_launch_kernel:
    ldr x1, =kernel_main
   
4:  blr x1
    wfe
    b       4b



.balign 4096
.global id_pg_dir
id_pg_dir:
    .space (3 * (1 << 12)), 0

.balign 4096
.global high_pg_dir
high_pg_dir:
    .space (6 * (1 << 12)), 0


.globl memzero
memzero:
    /* store 8 bytes of zero to [x0], then x0 += 8 */
    str xzr, [x0], #8
    subs x1, x1, #8
    /* branch if greater than zero */
    b.gt memzero
    ret



/* changes tmp1, tmp2 only */
.macro create_table_entry, tbl, ntbl, va, shift, flags, tmp1, tmp2
    /* get entry index in tmp1 */
    lsr \tmp1, \va, #\shift
    and \tmp1, \tmp1, #ENTRIES_PER_TABLE - 1
    /* tmp2 = entry value */
    mov \tmp2, \ntbl
    orr \tmp2, \tmp2, #\flags
    /* install entry */
    str \tmp2, [\tbl, \tmp1, lsl #3]
.endm

/* changes vstart, vend, pa, tmp1 */
/* vstart and vend should not point to the same block! */
.macro create_block_map, pmd, vstart, vend, pa, flags, tmp1
    /* turn vstart, vend into indices */
    lsr \vstart, \vstart, #SECTION_SHIFT
    and \vstart, \vstart, #ENTRIES_PER_TABLE - 1
    lsr \vend, \vend, #SECTION_SHIFT
    /* minus one to handle the last entry */
    sub \vend, \vend, #1
    and \vend, \vend, #ENTRIES_PER_TABLE - 1
    /* loop init, pa = pa | flags */
    lsr \pa, \pa, #SECTION_SHIFT
    lsl \pa, \pa, #SECTION_SHIFT
    ldr \tmp1, =\flags
    orr \pa, \pa, \tmp1
    /* loop */
    /* pmd[vstart] = pa */
2:
    str \pa, [\pmd, \vstart, lsl #3]
    /* pa += section size */
    add \pa, \pa, #SECTION_SIZE
    /* vstart += 1 */
    add \vstart, \vstart, #1
    cmp \vstart, \vend
    b.le 2b
.endm


map_identity:
    /* save return address */
    mov x29, x30
    adrp x0, id_pg_dir
    mov x1, #ID_MAP_TABLE_SIZE
    /* clear id page tables */
    bl memzero
    adrp x0, id_pg_dir
    /* x1 = address of id map pud */
    add x1, x0, #PAGE_SIZE
    eor x4, x4, x4
    /* install PGD entry */
    
    /* changes tmp1, tmp2 only */
    //.macro create_table_entry, tbl, ntbl, va, shift, flags, tmp1, tmp2
/*
tbl - a pointer to a memory region were new table has to be allocated.
virt - virtual address that we are currently mapping.
shift - shift that we need to apply to the virtual address in order to extract current table index. (39 in case of PGD and 30 in case of PUD)
tmp1, tmp2 - temporary registers.
 */
    create_table_entry x0, x1, x4, PGD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    /* goto next level */
    add x0, x0, #PAGE_SIZE
    add x1, x1, #PAGE_SIZE
    /* install PUD entry */
    create_table_entry x0, x1, x4, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    /* id map 0-16M */
    mov x0, x1
    eor x2, x2, x2
    ldr x3, =ID_MAP_SIZE
    eor x4, x4, x4
    create_block_map x0, x2, x3, x4, TD_KERNEL_BLOCK_FLAGS, x5
    
    /* restore return address */
    mov x30, x29
    ret


map_high:
    /* save return address */
    mov x29, x30
    adrp x0, high_pg_dir
    mov x1, #HIGH_MAP_TABLE_SIZE
    /* clear high page tables */
    bl memzero
    adrp x0, high_pg_dir
    /* x1 = address of high map pud */
    add x1, x0, #PAGE_SIZE
    /* x4 = address of va we map (pgd) */
    ldr x4, =LINEAR_MAP_BASE
    /* install PGD entry */
    create_table_entry x0, x1, x4, PGD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    /* goto next level */
    add x0, x0, #PAGE_SIZE
    add x1, x1, #PAGE_SIZE
    /* x4 = address of va we map (pud) */
    ldr x4, =LINEAR_MAP_BASE
    ldr x5, =PUD_ENTRY_MAP_SIZE
    /* install first PUD entry */
    create_table_entry x0, x1, x4, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    add x1, x1, #PAGE_SIZE
    add x4, x4, x5
    create_table_entry x0, x1, x4, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    add x1, x1, #PAGE_SIZE
    add x4, x4, x5
    create_table_entry x0, x1, x4, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    add x1, x1, #PAGE_SIZE
    add x4, x4, x5
    create_table_entry x0, x1, x4, PUD_SHIFT, TD_KERNEL_TABLE_FLAGS, x2, x3
    /* load some values */
    ldr x10, =HIGH_MAP_FIRST_START
    ldr x11, =HIGH_MAP_FIRST_END
    ldr x12, =HIGH_MAP_SECOND_START
    ldr x13, =HIGH_MAP_SECOND_END
    ldr x14, =HIGH_MAP_THIRD_START
    ldr x15, =HIGH_MAP_THIRD_END
    ldr x16, =HIGH_MAP_FOURTH_START
    ldr x17, =HIGH_MAP_FOURTH_END
    ldr x18, =HIGH_MAP_DEVICE_START
    ldr x19, =HIGH_MAP_DEVICE_END
    ldr x20, =FIRST_START
    ldr x21, =SECOND_START
    ldr x22, =THIRD_START
    ldr x23, =FOURTH_START
    ldr x24, =DEVICE_START
    /* map first high part */
    add x0, x0, #PAGE_SIZE
    mov x2, x10
    mov x3, x11
    mov x4, x20
    create_block_map x0, x2, x3, x4, TD_KERNEL_BLOCK_FLAGS, x5
    /* map second high part */
    add x0, x0, #PAGE_SIZE
    mov x2, x12
    mov x3, x13
    mov x4, x21
    create_block_map x0, x2, x3, x4, TD_KERNEL_BLOCK_FLAGS, x5
    /* map third high part */
    add x0, x0, #PAGE_SIZE
    mov x2, x14
    mov x3, x15
    mov x4, x22
    create_block_map x0, x2, x3, x4, TD_KERNEL_BLOCK_FLAGS, x5
    /* map fourth high part */
    add x0, x0, #PAGE_SIZE
    mov x2, x16
    mov x3, x17
    mov x4, x23
    create_block_map x0, x2, x3, x4, TD_KERNEL_BLOCK_FLAGS, x5
    /* map device */
    mov x2, x18
    mov x3, x19
    mov x4, x24
    create_block_map x0, x2, x3, x4, TD_DEVICE_BLOCK_FLAGS, x5
    
    /* restore return address */
    mov x30, x29
    ret


.section ".text"
__exception_restore_context:
	ldr	w19,      [sp, #16 * 16]
	ldp	lr,  x20, [sp, #16 * 15]

	msr	SPSR_EL1, x19
	msr	ELR_EL1,  x20

	ldp	x0,  x1,  [sp, #16 * 0]
	ldp	x2,  x3,  [sp, #16 * 1]
	ldp	x4,  x5,  [sp, #16 * 2]
	ldp	x6,  x7,  [sp, #16 * 3]
	ldp	x8,  x9,  [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]

	add	sp,  sp,  #16 * 17

	eret


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

/*
    IRQ Handlers
*/

irq_sp0:
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

    bl    irq_handler_sp0

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

irq_spx:
    sub	sp,  sp,  #16 * 17

	// Store all general purpose registers on the stack.
	stp	x0,  x1,  [sp, #16 * 0]
	stp	x2,  x3,  [sp, #16 * 1]
	stp	x4,  x5,  [sp, #16 * 2]
	stp	x6,  x7,  [sp, #16 * 3]
	stp	x8,  x9,  [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]

	// Add the exception link register (ELR_EL1), saved program status (SPSR_EL1) and exception
	// syndrome register (ESR_EL1).
	mrs	x1,  ELR_EL1
	mrs	x2,  SPSR_EL1
	mrs	x3,  ESR_EL1

	stp	lr,  x1,  [sp, #16 * 15]
	stp	x2,  x3,  [sp, #16 * 16]

	// x0 is the first argument for the function called through `\handler`.
	mov	x0,  sp


/* 
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

 */ bl    irq_handler_spx


    b __exception_restore_context
    /* ldp   x18, x19, [sp], #16
    ldp   x16, x17, [sp], #16
    ldp   x14, x15, [sp], #16
    ldp   x12, x13, [sp], #16
    ldp   x10, x11, [sp], #16
    ldp   x8,  x9,  [sp], #16
    ldp   x6,  x7,  [sp], #16
    ldp   x4,  x5,  [sp], #16
    ldp   x2,  x3,  [sp], #16
    ldp   x0,  x1,  [sp], #16
    eret */
    
irq_aarch64:
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

    bl    irq_handler_lower_aarch64

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
irq_aarch32:
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

    bl    irq_handler_lower_aarch32

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

/*
    Sync Handlers
*/
sync_sp0:
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

    bl     sync_handler_sp0

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


sync_spx:
   sub	sp,  sp,  #16 * 17

	// Store all general purpose registers on the stack.
	stp	x0,  x1,  [sp, #16 * 0]
	stp	x2,  x3,  [sp, #16 * 1]
	stp	x4,  x5,  [sp, #16 * 2]
	stp	x6,  x7,  [sp, #16 * 3]
	stp	x8,  x9,  [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]

	// Add the exception link register (ELR_EL1), saved program status (SPSR_EL1) and exception
	// syndrome register (ESR_EL1).
	mrs	x1,  ELR_EL1
	mrs	x2,  SPSR_EL1
	mrs	x3,  ESR_EL1
    mov x3, #31
	stp	lr,  x1,  [sp, #16 * 15]
	stp	x2,  x3,  [sp, #16 * 16]

	// x0 is the first argument for the function called through `\handler`.
	mov	x0,  sp

    bl     sync_handler_spx
    b __exception_restore_context

   
    eret


sync_aarch64:
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

    bl     sync_handler_lower_aarch64

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



sync_aarch32:
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

    bl     sync_handler_lower_aarch32

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


/*
    FIQ Handlers
*/
fiq_sp0:
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

    bl     fiq_handler_sp0

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

fiq_spx:
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

    bl     fiq_handler_spx

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
fiq_aarch64:
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

    bl     fiq_handler_lower_aarch64

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

fiq_aarch32:
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

    bl     fiq_handler_lower_aarch32

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

/*
    SError Handlers
*/
serror_sp0:
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

    bl     serror_handler_sp0

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


serror_spx:
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

    bl     serror_handler_spx

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

    serror_aarch64:
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

    bl     serror_handler_lower_aarch64

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

    serror_aarch32:
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

    bl     serror_handler_lower_aarch32

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
.global vector
vector:

//Current level w/ SP0
    //Sync 
.balign 128
    b sync_sp0
    // IRQ 
.balign 128
    b irq_sp0
    //FIQ
.balign 128
    b fiq_sp0
    //SError
.balign 128
    b serror_sp0

//Current EL with SPx
    //Sync 
.balign 128
    b sync_spx
    //IRQ
.balign 128
    b irq_spx
    //FIQ
.balign 128
    b fiq_spx
    //SError
.balign 128
    b serror_spx

//LowerEL ausing AArch64
    //Sync
.balign 128
    b sync_aarch64
    //IRQ
.balign 128
    b irq_aarch64
    //FIQ
.balign 128
    b fiq_aarch64
    //SError
.balign 128
    b serror_aarch64

//LowerEL ausing AArch32
    //Sync
.balign 128
    b sync_aarch32
    //IRQ
.balign 128
    b irq_aarch32
    //FIQ
.balign 128
    b fiq_aarch32
    //SError
.balign 128
    b serror_aarch32

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

 