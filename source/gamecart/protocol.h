// Copyright 2014 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once
#include "common.h"

#define REG_CARDCONF  (*(vu16*)0x1000000C)
#define REG_CARDCONF2 (*(vu8*)0x10000010)

//REG_AUXSPICNT
#define CARD_ENABLE     (1<<15)
#define CARD_SPI_ENABLE (1<<13)
#define CARD_SPI_BUSY   (1<<7)
#define CARD_SPI_HOLD   (1<<6)

// for CTRCARD, but GM9 uses it in NTRCARD init.
#define REG_CTRCARDSECCNT (*(vu32*)0x10004008)

#define LATENCY 0x822C

void Cart_Init(void);
uint32_t Cart_GetID(void);
void SwitchToNTRCARD(void);
void ResetCartSlot(void);
