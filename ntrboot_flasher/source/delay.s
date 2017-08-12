// Copyright 2014 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

.arm
.global ioDelay
.type   ioDelay STT_FUNC

@ioDelay ( u32 us )
ioDelay:
	subs r0, #1
	bgt ioDelay
	bx lr
	
.arm
.global testShit
.type testShit STT_FUNC

testShit:
	push {r0-r8, lr}
	ldr r4, =0x08001000
	cmp r4, #0
	.word 0x9D44D74E
	pop {r0-r8, lr}
	bx lr
	
	.pool
	