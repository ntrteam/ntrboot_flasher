@---------------------------------------------------------------------------------
	.section ".crt0","ax"
	.global _start
@---------------------------------------------------------------------------------
	.global _start
	.global _init
	.align	4
	.arm
    .extern _Z24InstallExceptionHandlersv
@---------------------------------------------------------------------------------
_start:
@---------------------------------------------------------------------------------
	@move SP to arm9 memory to improve performance and to allow more fcram usage
	ldr sp, =0x27F00000

	push {lr}
	push {r0-r1}
	
	mrs r0, cpsr
	orr r0, #(1<<7) @Disable IRQ
	msr cpsr_c, r0
	
	bl cp15_start

	ldr	r1, =__itcm_lma		@ Copy instruction tightly coupled memory (itcm section) from LMA to VMA
	ldr	r2, =__itcm_start
	ldr	r4, =__itcm_end
	bl	CopyMemCheck

	ldr	r1, =__dtcm_lma		@ Copy data tightly coupled memory (dtcm section) from LMA to VMA
	ldr	r2, =__dtcm_start
	ldr	r4, =__dtcm_end
	bl	CopyMemCheck
	
	ldr	r0, =__bss_start__	@ Clear BSS section
	ldr	r1, =__bss_end__
	sub	r1, r1, r0
	bl	ClearMem
	
	ldr	r0, =__sbss_start	@ Clear SBSS section 
	ldr	r1, =__sbss_end
	sub	r1, r1, r0
	bl	ClearMem
	
@---------------------------------------------------------------------------------
@ global constructors
@---------------------------------------------------------------------------------
	ldr		r3,=__libc_init_array
	blx		r3
@---------------------------------------------------------------------------------
@ Jump to user code
@---------------------------------------------------------------------------------
_call_main:
@---------------------------------------------------------------------------------
	pop {r0-r1}
	ldr		r3, =main
	blx		r3
	
	pop {pc}
	
.global cp15_start
.type cp15_start STT_FUNC
cp15_start:
	PUSH    {R0-R12, LR}
	LDR     R0, =0x2078
	MCR     p15, 0, R0,c1,c0, 0
	LDR     R0, =0xFFF00027			@BIOS 	(0x100000)
	LDR     R1, =0x1FF801D			@ITCM 	(0x8000)
	LDR     R2, =0x8000027			@RAM 	(0x100000)
	LDR     R3, =0x10000025			@IO 	(0x80000)
	LDR     R4, =0x10100027			@IO 	(0x100000)
	LDR     R5, =0x20000037			@FCRAM 	(0x10000000)
	LDR     R6, =0x3000801D			@DTCM 	(0x8000)
	LDR     R7, =0x18000035			@VRAM 	(0x8000000)
	LDR     R8, =0x33333333
	LDR     R9, =0x60600663
	MOV     R10, #0xA5
	MOV     R11, #0xA5
	MOV     R12, #0xA5
	MCR     p15, 0, R0,c6,c0, 0
	MCR     p15, 0, R1,c6,c1, 0
	MCR     p15, 0, R2,c6,c2, 0
	MCR     p15, 0, R3,c6,c3, 0
	MCR     p15, 0, R4,c6,c4, 0
	MCR     p15, 0, R5,c6,c5, 0
	MCR     p15, 0, R6,c6,c6, 0
	MCR     p15, 0, R7,c6,c7, 0
	MCR     p15, 0, R8,c5,c0, 2
	MCR     p15, 0, R8,c5,c0, 3
	MCR     p15, 0, R10,c3,c0, 0
	MCR     p15, 0, R11,c2,c0, 0
	MCR     p15, 0, R12,c2,c0, 1
	LDR     R0, =0x3000800A 		@DTCM 	(0x4000)
	MCR     p15, 0, R0,c9,c1, 0
	MOV     R0, #0x20 				@ITCM 	(0x2000000)
	MCR     p15, 0, R0,c9,c1, 1
	BL		clean_and_flush_full_data_cache_by_index_drain_write_buffer
	BL		flush_full_instruction_cache
	LDR     R0, =0x5307D
	MCR     p15, 0, R0,c1,c0, 0
	POP     {R0-R12, PC}
crash_exception_vectors:
	B       crash_exception_vectors
.pool

.global clean_and_flush_full_data_cache_by_index_drain_write_buffer
.type clean_and_flush_full_data_cache_by_index_drain_write_buffer STT_FUNC
clean_and_flush_full_data_cache_by_index_drain_write_buffer:
    MOV     R12, #0
loc_809AC74:
    MOV     R0, #0
    MOV     R2, R12,LSL#30
loc_809AC7C:
    ORR     R1, R2, R0,LSL#5
    MCR     p15, 0, R1,c7,c14, 2
    ADD     R0, R0, #1
    CMP     R0, #0x20
    BCC     loc_809AC7C
    ADD     R12, R12, #1
    CMP     R12, #4
    BCC     loc_809AC74
    MOV     R0, #0
    MCR     p15, 0, R0,c7,c10, 4
    BX      LR

.global flush_full_instruction_cache
.type flush_full_instruction_cache STT_FUNC
flush_full_instruction_cache:
    MOV     R0, #0
    MCR     p15, 0, R0,c7,c5, 0
    BX      LR
	
@---------------------------------------------------------------------------------
@ Clear memory to 0x00 if length != 0
@  r0 = Start Address
@  r1 = Length
@---------------------------------------------------------------------------------
ClearMem:
@---------------------------------------------------------------------------------
	mov	r2, #3			@ Round down to nearest word boundary
	add	r1, r1, r2		@ Shouldn't be needed
	bics	r1, r1, r2	@ Clear 2 LSB (and set Z)
	bxeq	lr			@ Quit if copy size is 0
 
	mov	r2, #0
ClrLoop:
	stmia	r0!, {r2}
	subs	r1, r1, #4
	bne	ClrLoop
 
	bx	lr
	
@---------------------------------------------------------------------------------
@ Copy memory if length	!= 0
@  r1 = Source Address
@  r2 = Dest Address
@  r4 = Dest Address + Length
@---------------------------------------------------------------------------------
CopyMemCheck:
@---------------------------------------------------------------------------------
	sub	r3, r4, r2		@ Is there any data to copy?
@---------------------------------------------------------------------------------
@ Copy memory
@  r1 = Source Address
@  r2 = Dest Address
@  r3 = Length
@---------------------------------------------------------------------------------
CopyMem:
@---------------------------------------------------------------------------------
	mov	r0, #3			@ These commands are used in cases where
	add	r3, r3, r0		@ the length is not a multiple of 4,
	bics	r3, r3, r0	@ even though it should be.
	bxeq	lr			@ Length is zero, so exit
CIDLoop:
	ldmia	r1!, {r0}
	stmia	r2!, {r0}
	subs	r3, r3, #4
	bne	CIDLoop

	bx	lr
