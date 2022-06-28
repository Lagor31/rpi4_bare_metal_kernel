SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 */
    .text : { KEEP(*(.text.boot)) *(.text .text.* .gnu.linkonce.t*) }
    .rodata : { *(.rodata .rodata.* .gnu.linkonce.r*) }
    PROVIDE(_data = .);
    .data : { *(.data .data.* .gnu.linkonce.d*) }
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
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}
__bss_size = (__bss_end - __bss_start)>>3;
