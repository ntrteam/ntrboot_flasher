@---------------------------------------------------------------------------------
    .section ".crt0","ax"
    .global _start
@---------------------------------------------------------------------------------
    .global _start
    .global _init
    .align  4
    .arm
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
    msr cpsr_cx, #0xD3         @ switch to supervisor mode, disable interrupts
    ldr sp, =0x27F00000

    mov r9, r0
    mov r10, r1
    lsl r2, #0x10
    lsr r2, #0x10
    ldr r4, =0xBEEF
    cmp r2, r4
    movne r9, #0

    @ Disable caches / MPU
    mrc p15, 0, r4, c1, c0, 0  @ read control register
    bic r4, #(1<<16)           @ - DTCM disable
    bic r4, #(1<<12)           @ - instruction cache disable
    bic r4, #(1<<2)            @ - data cache disable
    bic r4, #(1<<0)            @ - MPU disable
    mcr p15, 0, r4, c1, c0, 0  @ write control register

    @ Invalidate both caches, discarding any data they may contain,
    @ then drain the write buffer
    mov r4, #0
    mcr p15, 0, r4, c7, c5, 0
    mcr p15, 0, r4, c7, c6, 0
    mcr p15, 0, r4, c7, c10, 4

    @ Give read/write access to all the memory regions
    ldr r0, =0x33333333
    mcr p15, 0, r0, c5, c0, 2 @ write data access
    mcr p15, 0, r0, c5, c0, 3 @ write instruction access

    @ Set MPU permissions and cache settings
    ldr r0, =0xFFFF001D @ ffff0000 32k  | bootrom (unprotected part)
    ldr r1, =0x3000801B @ 30008000 16k  | dtcm
    ldr r2, =0x01FF801D @ 01ff8000 32k  | itcm
    ldr r3, =0x08000027 @ 08000000 1M   | arm9 mem
    ldr r4, =0x10000029 @ 10000000 2M   | io mem (ARM9 / first 2MB)
    ldr r5, =0x20000035 @ 20000000 128M | fcram
    ldr r6, =0x1FF00027 @ 1FF00000 1M   | dsp / axi wram
    ldr r7, =0x1800002D @ 18000000 8M   | vram (+ 2MB)
    mov r8, #0x29
    mcr p15, 0, r0, c6, c0, 0
    mcr p15, 0, r1, c6, c1, 0
    mcr p15, 0, r2, c6, c2, 0
    mcr p15, 0, r3, c6, c3, 0
    mcr p15, 0, r4, c6, c4, 0
    mcr p15, 0, r5, c6, c5, 0
    mcr p15, 0, r6, c6, c6, 0
    mcr p15, 0, r7, c6, c7, 0
    mcr p15, 0, r8, c3, c0, 0   @ Write bufferable 0, 3, 5
    mcr p15, 0, r8, c2, c0, 0   @ Data cacheable 0, 3, 5
    mcr p15, 0, r8, c2, c0, 1   @ Inst cacheable 0, 3, 5

    @ Set DTCM address and size
    ldr r1, =0x3000800A        @ set DTCM address and size
    mcr p15, 0, r1, c9, c1, 0  @ set the dtcm Region Register

    @ Set ITC address and size
    ldr r1, =0x20              @ set DTCM address and size
    mcr p15, 0, r1, c9, c1, 0  @ set the dtcm Region Register

    @ Enable caches / MPU / ITCM
    mrc p15, 0, r0, c1, c0, 0  @ read control register
    orr r0, r0, #(1<<18)       @ - ITCM enable
    orr r0, r0, #(1<<16)       @ - DTCM enable
    orr r0, r0, #(1<<13)       @ - alternate exception vectors enable
    orr r0, r0, #(1<<12)       @ - instruction cache enable
    orr r0, r0, #(1<<2)        @ - data cache enable
    orr r0, r0, #(1<<0)        @ - MPU enable
    mcr p15, 0, r0, c1, c0, 0  @ write control register

    ldr r1, =__itcm_lma        @ Copy instruction tightly coupled memory (itcm section) from LMA to VMA
    ldr r2, =__itcm_start
    ldr r4, =__itcm_end
    bl CopyMemCheck

    ldr r1, =__dtcm_lma        @ Copy data tightly coupled memory (dtcm section) from LMA to VMA
    ldr r2, =__dtcm_start
    ldr r4, =__dtcm_end
    bl CopyMemCheck
    
    ldr r0, =__bss_start__     @ Clear BSS section
    ldr r1, =__bss_end__
    sub r1, r1, r0
    bl ClearMem

    ldr r0, =__sbss_start      @ Clear SBSS section 
    ldr r1, =__sbss_end
    sub r1, r1, r0
    bl ClearMem

@---------------------------------------------------------------------------------
@ global constructors
@---------------------------------------------------------------------------------
    ldr r3,=__libc_init_array
    blx r3
@---------------------------------------------------------------------------------
@ Jump to user code
@---------------------------------------------------------------------------------
_call_main:
@---------------------------------------------------------------------------------
    mov r0, r9
    mov r1, r10
    ldr r3, =main
    blx r3

    bkpt 0xABCD

@---------------------------------------------------------------------------------
@ Clear memory to 0x00 if length != 0
@  r0 = Start Address
@  r1 = Length
@---------------------------------------------------------------------------------
ClearMem:
@---------------------------------------------------------------------------------
    mov r2, #3          @ Round down to nearest word boundary
    add r1, r1, r2      @ Shouldn't be needed
    bics r1, r1, r2     @ Clear 2 LSB (and set Z)
    bxeq lr             @ Quit if copy size is 0

    mov	r2, #0
ClrLoop:
    stmia r0!, {r2}
    subs r1, r1, #4
    bne	ClrLoop

    bx lr

@---------------------------------------------------------------------------------
@ Copy memory if length	!= 0
@  r1 = Source Address
@  r2 = Dest Address
@  r4 = Dest Address + Length
@---------------------------------------------------------------------------------
CopyMemCheck:
@---------------------------------------------------------------------------------
    sub r3, r4, r2      @ Is there any data to copy?
@---------------------------------------------------------------------------------
@ Copy memory
@  r1 = Source Address
@  r2 = Dest Address
@  r3 = Length
@---------------------------------------------------------------------------------
CopyMem:
@---------------------------------------------------------------------------------
    mov r0, #3          @ These commands are used in cases where
    add r3, r3, r0      @ the length is not a multiple of 4,
    bics r3, r3, r0     @ even though it should be.
    bxeq lr             @ Length is zero, so exit
CIDLoop:
    ldmia r1!, {r0}
    stmia r2!, {r0}
    subs r3, r3, #4
    bne CIDLoop

    bx lr
