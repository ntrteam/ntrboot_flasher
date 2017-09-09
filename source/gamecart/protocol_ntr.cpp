// Copyright 2014 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "protocol_ntr.h"
#include "delay.h"

void NTR_SendCommand(const u8 command[8], u32 pageSize, u32 latency, void* buffer)
{
    REG_NTRCARDMCNTH = NTRCARD_CR1_ENABLE | NTRCARD_CR1_IRQ;

	for( u32 i=0; i<8; ++i )
        REG_NTRCARDCMD[i] = command[i];

    pageSize -= pageSize & 3; // align to 4 byte

    u32 pageParam = NTRCARD_PAGESIZE_4K;
    u32 transferLength = 4096;

    // make zero read and 4 byte read a little special for timing optimization(and 512 too)
    switch (pageSize) {
        case 0:
            transferLength = 0;
            pageParam = NTRCARD_PAGESIZE_0;
            break;
        case 4:
            transferLength = 4;
            pageParam = NTRCARD_PAGESIZE_4;
            break;
        case 512:
            transferLength = 512;
            pageParam = NTRCARD_PAGESIZE_512;
            break;
        case 8192:
            transferLength = 8192;
            pageParam = NTRCARD_PAGESIZE_8K;
            break;
        case 16384:
            transferLength = 16384;
            pageParam = NTRCARD_PAGESIZE_16K;
            break;
        default:
            break; // 4K pagesize default
    }

    REG_NTRCARDROMCNT = 0x10000000;
    REG_NTRCARDROMCNT = NTRKEY_PARAM | NTRCARD_ACTIVATE | NTRCARD_nRESET | pageParam | latency;

    u8 * pbuf = (u8 *)buffer;
    u32 * pbuf32 = (u32 * )buffer;
    bool useBuf = ( nullptr != pbuf );
    bool useBuf32 = (useBuf && (0 == (3 & ((u32)buffer))));

    u32 count = 0;
    u32 cardCtrl = REG_NTRCARDROMCNT;

    if(useBuf32)
    {
        do
        {
            cardCtrl = REG_NTRCARDROMCNT;
            if( cardCtrl & NTRCARD_DATA_READY  ) {
                u32 data = REG_NTRCARDFIFO;
                *pbuf32++ = data;
                count += 4;
            }
        } while( (cardCtrl & NTRCARD_BUSY) && count < pageSize);
    }
    else if(useBuf)
    {
        do
        {
            cardCtrl = REG_NTRCARDROMCNT;
            if( cardCtrl & NTRCARD_DATA_READY  ) {
                u32 data = REG_NTRCARDFIFO;
                pbuf[0] = (unsigned char) (data >>  0);
                pbuf[1] = (unsigned char) (data >>  8);
                pbuf[2] = (unsigned char) (data >> 16);
                pbuf[3] = (unsigned char) (data >> 24);
                pbuf += sizeof (unsigned int);
                count += 4;
            }
        } while( (cardCtrl & NTRCARD_BUSY) && count < pageSize);
    }
    else
    {
        do
        {
            cardCtrl = REG_NTRCARDROMCNT;
            if( cardCtrl & NTRCARD_DATA_READY  ) {
                u32 data = REG_NTRCARDFIFO;
                (void)data;
                count += 4;
            }
        } while( (cardCtrl & NTRCARD_BUSY) && count < pageSize);
    }

    // if read is not finished, ds will not pull ROM CS to high, we pull it high manually
    if( count != transferLength ) {
        // MUST wait for next data ready,
        // if ds pull ROM CS to high during 4 byte data transfer, something will mess up
        // so we have to wait next data ready
        do { cardCtrl = REG_NTRCARDROMCNT; } while(!(cardCtrl & NTRCARD_DATA_READY));
        // and this tiny delay is necessary
        //ioAK2Delay(33);
        // pull ROM CS high
        REG_NTRCARDROMCNT = 0x10000000;
        REG_NTRCARDROMCNT = NTRKEY_PARAM | NTRCARD_ACTIVATE | NTRCARD_nRESET;
    }
    // wait rom cs high
    do { cardCtrl = REG_NTRCARDROMCNT; } while( cardCtrl & NTRCARD_BUSY );
    //lastCmd[0] = command[0];lastCmd[1] = command[1];
}


void NTR_CmdReset(void)
{
    //cardReset ();
    ioDelay(0xF000);
}

u32 NTR_CmdGetCartId(void)
{
    return 0;
    //return cardReadID (0);
}

void NTR_CmdReadHeader (u8* buffer)
{
	REG_NTRCARDROMCNT=0;
	REG_NTRCARDMCNT=0;
	ioDelay(167550);
	REG_NTRCARDMCNT=NTRCARD_CR1_ENABLE|NTRCARD_CR1_IRQ;
	REG_NTRCARDROMCNT=NTRCARD_nRESET|NTRCARD_SEC_SEED;
	while(REG_NTRCARDROMCNT&NTRCARD_BUSY) ;
	//cardReset();
	while(REG_NTRCARDROMCNT&NTRCARD_BUSY) ;
	/*u32 iCardId=cardReadID(NTRCARD_CLK_SLOW);
	while(REG_NTRCARDROMCNT&NTRCARD_BUSY) ;
	
	u32 iCheapCard=iCardId&0x80000000;
	
    if(iCheapCard)
    {
      //this is magic of wood goblins
      for(size_t ii=0;ii<8;++ii)
        cardParamCommand(NTRCARD_CMD_HEADER_READ,ii*0x200,NTRCARD_ACTIVATE|NTRCARD_nRESET|NTRCARD_CLK_SLOW|NTRCARD_BLK_SIZE(1)|NTRCARD_DELAY1(0x1FFF)|NTRCARD_DELAY2(0x3F),(u32*)(void*)(buffer+ii*0x200),0x200/sizeof(u32));
    }
    else
    {
      //0xac3f1fff
      cardParamCommand(NTRCARD_CMD_HEADER_READ,0,NTRCARD_ACTIVATE|NTRCARD_nRESET|NTRCARD_CLK_SLOW|NTRCARD_BLK_SIZE(4)|NTRCARD_DELAY1(0x1FFF)|NTRCARD_DELAY2(0x3F),(u32*)(void*)buffer,0x1000/sizeof(u32));
    }*/
    //cardReadHeader (buffer);
}

void NTR_CmdReadData (u32 offset, void* buffer)
{
    //cardParamCommand (NTRCARD_CMD_DATA_READ, offset, ReadDataFlags | NTRCARD_ACTIVATE | NTRCARD_nRESET | NTRCARD_BLK_SIZE(1), (u32*)buffer, 0x200 / 4);
}
