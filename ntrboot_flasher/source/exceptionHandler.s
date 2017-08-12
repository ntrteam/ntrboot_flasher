
.arm

.extern ClearExceptionScreen
.extern DrawExceptionData
.extern EndExceptionScreen

	.global	dataAbort
	.type	dataAbort STT_FUNC
	dataAbort:
		STMFD   SP!, {R0-R15}^
		STMFD   SP!, {R0-R15}
		ldr r2, =0xDA7A
		b guruMeditation
		
	.global	prefetchAbort
	.type	prefetchAbort STT_FUNC
	prefetchAbort:
		STMFD   SP!, {R0-R15}^
		STMFD   SP!, {R0-R15}
		ldr r2, =0xBABE
		b guruMeditation

	.global	guruMeditation
	.type	guruMeditation STT_FUNC
	guruMeditation:
	.arm		
		mov r10, r2
		mov r11, sp
		ldr sp,=0x22170000
		
		bl ClearExceptionScreen

		mov r0, #0
		mov r1, #0
		mov r2, r10
		bl DrawExceptionData

		mov r10, #0
		guruDumploop_:
			mov r0, #8
			mul r0, r10
			add r0, #20
			mov r1, #0
			ldr r2, [r11, r10, lsl #2]
			bl DrawExceptionData

			add r10, #1
			cmp r10, #16
			blt guruDumploop_

		add r11, r10, lsl #2
		mov r10, #0
		guruDumploop_2:
			mov r0, #8
			mul r0, r10
			add r0, #20
			mov r1, #80
			ldr r2, [r11, r10, lsl #2]
			bl DrawExceptionData

			add r10, #1
			cmp r10, #16
			blt guruDumploop_2

		guruMeditationLoop:
			bl EndExceptionScreen
			
.pool
		