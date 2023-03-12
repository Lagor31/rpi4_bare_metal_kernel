/*
 * raspi/start32.S
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

.section ".text.boot"

.global _start

_start:
    // save arguments on stack (we will need them later for the new kernel)
    ldr     sp, =_start
    push    { r0, r1, r2, r3 }

    // relocate our code from load address to link address
    ldr     r1, =LOAD_ADDR
    mov     r0, r1
    ldr     r2, =_start
    ldr     r3, =__loader_size
1:  ldr     r4, [r1], #4
    str     r4, [r2], #4
    subs    r3, #1
    bne     1b

    // jump to relocated C code
    bl      receiver-1024
    // restore registers and jump to new kernel's entry point
    mov     lr, r0
    pop     { r0, r1, r2, r3 }
    bx      lr
