SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 (VMA)*/
    .boot : {
        KEEP(*(.boot))
    }

    /* . = 0xFFFF000000000000; */
    /* AT(0x80000 + SIZEOF(.boot) ) */
    .text :  { 
         KEEP(*(.text)) 
        *(.text .text.* .gnu.linkonce.t*) 
    } 

    .rodata : { 
        *(.rodata .rodata.* .gnu.linkonce.r*) 
    }

    PROVIDE(_data = .);

    .data : { 
        *(.data .data.* .gnu.linkonce.d*) 
    }

    .bss (NOLOAD) : {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }

    .boot_alloc (NOLOAD) : {
        . = ALIGN(16);
        _boot_alloc_start = .;
        *(.boot_alloc .boot_alloc.*)
        *(COMMON)
        _boot_alloc_end = .;
    }

    . = _boot_alloc_end;

    .heap (NOLOAD) : {
        . = ALIGN(16);
        _heap_start = .;
        *(.heap .heap.*)
        *(COMMON)
        _heap_end = .;
    }

    . = _heap_end;

    . = . + 512;
    . = ALIGN(16);
    __stack_start  = .;

    .cpu1Stack (NOLOAD) :    {
        . = . + 512;
        __cpu1_stack  = .;
    }

    . = ALIGN(16);
    .cpu2Stack (NOLOAD):    {
        . = . + 512;
        __cpu2_stack  = .;
    }

    . = ALIGN(16);
    .cpu3Stack (NOLOAD):    {
        . = . + 512;
        __cpu3_stack  = .;
    }

    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_size = (__bss_end - __bss_start)>>3;
