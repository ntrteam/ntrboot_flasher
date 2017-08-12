/*
    exceptions.cpp
    Copyright (C) 2015 Normmatt
*/

#include <stdint.h>
#include "DrawCharacter.h"
#include "misc.h"
#include "i2c.h"

extern "C" void dataAbort();
extern "C" void prefetchAbort();

extern "C" void ClearExceptionScreen();
extern "C" void EndExceptionScreen();
extern "C" void DrawExceptionData(uint32_t y, uint32_t x, uint32_t val);

void ClearExceptionScreen()
{
	ClearScreen(TOP_SCREEN, RGB(0, 0, 0));
}

void DrawExceptionData(uint32_t y, uint32_t x, uint32_t val)
{
	DrawHex(TOP_SCREEN, val, x, y, RGB(255, 255, 255), RGB(255, 0, 0));
}

void EndExceptionScreen()
{
	while(1)
	{		
		uint32_t pad = KEYS;
		if(!(pad & KEY_B))
		{
			break;
		}
	}
	
	while(!i2cWriteRegister(I2C_DEV_MCU, I2CREGPM_BATTERY, 1));
}

static void ex_DataAbort(void)
{
    asm volatile ("SUB lr,lr,#4 @handler entry code \t\n");
    asm volatile ("STMFD sp!,{lr} \t\n");
    asm volatile ("LDMFD sp!,{pc}^ @ handler exit code \t\n");
}

static void __attribute__ ((interrupt ("ABORT"))) 
ex_PrefetchAbort(void)
{
	//DrawHex(TOP_SCREEN, 0xB00B135, 80, 120, RGB(255, 255, 255), RGB(255, 0, 255));
	//while(1);
}

static void __attribute__ ((interrupt ("UNDEF")))
ex_UndefinedAbort(void)
{
	DrawHex(TOP_SCREEN, 0xBEEFDADE, 80, 120, RGB(255, 255, 255), RGB(255, 0, 255));
	EndExceptionScreen();
}

static void __attribute__ ((interrupt ("FIQ")))
ex_Fiq(void)
{
	DrawHex(TOP_SCREEN, 0xF100F100, 80, 120, RGB(255, 255, 255), RGB(255, 0, 0));
	EndExceptionScreen();
}

void InstallExceptionHandlers()
{
	*(uint32_t*)0x0800000C = (uint32_t)ex_Fiq;
	*(uint32_t*)0x0800001C = (uint32_t)ex_UndefinedAbort;
	*(uint32_t*)0x08000024 = (uint32_t)prefetchAbort;
	*(uint32_t*)0x0800002C = (uint32_t)dataAbort;
}