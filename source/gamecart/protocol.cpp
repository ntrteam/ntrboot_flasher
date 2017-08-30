// Copyright 2014 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "protocol.h"
#include "protocol_ntr.h"

#include "delay.h"

uint32_t CartID = 0xFFFFFFFFu;

void ResetCartSlot(void)
{
    REG_CARDCONF2 = 0x0C;
    REG_CARDCONF &= ~3;

    if (REG_CARDCONF2 == 0xC) {
        while (REG_CARDCONF2 != 0);
    }

    if (REG_CARDCONF2 != 0)
        return;

    REG_CARDCONF2 = 0x4;
    while(REG_CARDCONF2 != 0x4);

    REG_CARDCONF2 = 0x8;
    while(REG_CARDCONF2 != 0x8);
}

void SwitchToNTRCARD(void)
{
    REG_NTRCARDROMCNT |= 0x20000000;
    REG_CARDCONF &= ~3;
    REG_CARDCONF &= ~0x100;
    REG_NTRCARDMCNTH = NTRCARD_CR1_ENABLE;
}

uint32_t Cart_GetID(void)
{
    return CartID;
}

void Cart_Reset(void)
{
    ResetCartSlot(); //Seems to reset the cart slot?

    REG_CTRCARDSECCNT &= 0xFFFFFFFB;
    ioDelay(0x40000);

    SwitchToNTRCARD();
    ioDelay(0x40000);
}

void Cart_Init(void)
{
	ResetCartSlot(); //Seems to reset the cart slot?

    REG_CTRCARDSECCNT &= 0xFFFFFFFB;
    ioDelay(0x40000);

	SwitchToNTRCARD();
    ioDelay(0x40000);

    REG_NTRCARDROMCNT = 0;
    REG_NTRCARDMCNT = 0;
    ioDelay(0x40000);

    REG_NTRCARDMCNTH = (NTRCARD_CR1_ENABLE | NTRCARD_CR1_IRQ);
    REG_NTRCARDROMCNT = NTRCARD_nRESET | NTRCARD_SEC_SEED;
    while (REG_NTRCARDROMCNT & NTRCARD_BUSY);

    NTR_CmdReset();
    ioDelay(0x40000);
    CartID = NTR_CmdGetCartId();
}
