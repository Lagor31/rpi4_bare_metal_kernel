SECTIONS
{
    . = 0x80000;     /* Kernel load address for AArch64 (VMA)*/
    .boot : {
        KEEP(*(.boot))
        *(.boot_data*)
    }

    . = 0xFFFF000000000000 + 0x90000;
    .text :  AT(0x90000)  { 
         KEEP(*(.text)) 
        *(.text .text.* .gnu.linkonce.t*) 
    } 

    . = ALIGN(0x1000);
    .rodata :  ALIGN(0x1000){ 
        *(.rodata .rodata.* .gnu.linkonce.r*) 
    }

    . = ALIGN(0x1000);

    PROVIDE(_data = .);

    .data :ALIGN(0x1000) { 
        *(.data .data.* .gnu.linkonce.d*) 
    }
    . = ALIGN(0x1000);

    .bss (NOLOAD) :ALIGN(0x1000) {
        . = ALIGN(16);
        __bss_start = .;
        *(.bss .bss.*)
        *(COMMON)
        __bss_end = .;
    }
    . = ALIGN(0x1000);


    .boot_alloc (NOLOAD) : ALIGN(0x1000){
        . = ALIGN(16);
        _boot_alloc_start = .;
        *(.boot_alloc .boot_alloc.*)
        *(COMMON) 
        _boot_alloc_end = .;
    }

    . = _boot_alloc_end;
    . = ALIGN(0x1000);

    __stack_start  = .;
    . = . + 4096;

    /* Grows Downwards! */
    . = ALIGN(16);
    .cpu0Stack (NOLOAD) : {
        __cpu0_stack = . ;
    }

    . = ALIGN(16);
    .cpu1Stack (NOLOAD) :    {
        . = . + 4096;
        __cpu1_stack  = .;
    }

    . = ALIGN(16);
    .cpu2Stack (NOLOAD):    {
        . = . + 4096;
        __cpu2_stack  = .;
    }

    . = ALIGN(16);
    .cpu3Stack (NOLOAD):    {
        . = . + 4096;
        __cpu3_stack  = .;
    }
    
    . = ALIGN(0x1000);
    .heap (NOLOAD) : {
        . = ALIGN(16);
        _heap_start = .;
        *(.heap .heap.*)
        *(COMMON)
        _heap_end = .;
    }

    . = _heap_end;
    _end = .;

   /DISCARD/ : { *(.comment) *(.gnu*) *(.note*) *(.eh_frame*) }
}

__bss_size = (__bss_end - __bss_start)>>3;
