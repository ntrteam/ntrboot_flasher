/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2014-2015, Normmatt
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 2, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 2 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>

#include "sdmmc.h"
//#include "DrawCharacter.h"

//Uncomment to enable 32bit fifo support?
//not currently working
#define DATA32_SUPPORT

#define TRUE 1
#define FALSE 0

#define bool int

#define NO_INLINE __attribute__ ((noinline))

#define RGB(r,g,b) (r<<24|b<<16|g<<8|r)

#ifdef __cplusplus
extern "C" {
#endif
	void waitcycles(uint32_t val);
#ifdef __cplusplus
};
#endif

//#define DEBUG_SDMMC // Doesn't work anyway...

#ifdef DEBUG_SDMMC
	extern uint8_t* topScreen;
	extern void DrawHexWithName(unsigned char *screen, const char *str, unsigned int hex, int x, int y, int color, int bgcolor);
	#define DEBUGPRINT(scr,str,hex,x,y,color,bg) DrawHexWithName(scr,str,hex,x,y,color,bg)
#else
	#define DEBUGPRINT(...)
#endif

//extern "C" void sdmmc_send_command(struct mmcdevice *ctx, uint32_t cmd, uint32_t args);
//extern "C" void inittarget(struct mmcdevice *ctx);
//extern "C" int SD_Init();
//extern "C" int SD_Init2();
//extern "C" int Nand_Init2();
//extern "C" void InitSD();

struct mmcdevice handleNAND;
struct mmcdevice handleSD;

mmcdevice *getMMCDevice(int drive)
{
	if(drive==0) return &handleNAND;
	return &handleSD;
}

int geterror(struct mmcdevice *ctx)
{
	return (ctx->error << 29) >> 31;
}


void inittarget(struct mmcdevice *ctx)
{
	sdmmc_mask16(REG_SDPORTSEL,0x3,(uint16_t)ctx->devicenumber);
	setckl(ctx->clk);
	if(ctx->SDOPT == 0)
	{
		sdmmc_mask16(REG_SDOPT,0,0x8000);
	}
	else
	{
		sdmmc_mask16(REG_SDOPT,0x8000,0);
	}

}


void NO_INLINE sdmmc_send_command(struct mmcdevice *ctx, uint32_t cmd, uint32_t args)
{
	bool getSDRESP = (cmd << 15) >> 31;
	uint16_t flags = (cmd << 15) >> 31;
	const bool readdata = cmd & 0x20000;
	const bool writedata = cmd & 0x40000;

	if(readdata || writedata)
	{
		flags |= TMIO_STAT0_DATAEND;
	}

	ctx->error = 0;
	while((sdmmc_read16(REG_SDSTATUS1) & TMIO_STAT1_CMD_BUSY)); //mmc working?
	sdmmc_write16(REG_SDIRMASK0,0);
	sdmmc_write16(REG_SDIRMASK1,0);
	sdmmc_write16(REG_SDSTATUS0,0);
	sdmmc_write16(REG_SDSTATUS1,0);
#ifdef DATA32_SUPPORT
//	if(readdata)sdmmc_mask16(REG_DATACTL32, 0x1000, 0x800);
//	if(writedata)sdmmc_mask16(REG_DATACTL32, 0x800, 0x1000);
//	sdmmc_mask16(REG_DATACTL32,0x1800,2);
#else
	sdmmc_mask16(REG_DATACTL32,0x1800,0);
#endif
	sdmmc_write16(REG_SDCMDARG0,args &0xFFFF);
	sdmmc_write16(REG_SDCMDARG1,args >> 16);
	sdmmc_write16(REG_SDCMD,cmd &0xFFFF);

	uint32_t size = ctx->size;
	uint16_t *dataPtr = (uint16_t*)ctx->data;
	uint32_t *dataPtr32 = (uint32_t*)ctx->data;

	bool useBuf = ( NULL != dataPtr );
	bool useBuf32 = (useBuf && (0 == (3 & ((uint32_t)dataPtr))));

	uint16_t status0 = 0;
	while(1)
	{
		volatile uint16_t status1 = sdmmc_read16(REG_SDSTATUS1);
#ifdef DATA32_SUPPORT
		volatile uint16_t ctl32 = sdmmc_read16(REG_DATACTL32);
		if((ctl32 & 0x100))
#else
		if((status1 & TMIO_STAT1_RXRDY))
#endif
		{
			if(readdata)
			{
				if(useBuf)
				{
					sdmmc_mask16(REG_SDSTATUS1, TMIO_STAT1_RXRDY, 0);
					//sdmmc_write16(REG_SDSTATUS1,~TMIO_STAT1_RXRDY);
					if(size > 0x1FF)
					{
						#ifdef DATA32_SUPPORT
						if(useBuf32)
						{
							for(int i = 0; i<0x200; i+=4)
							{
								*dataPtr32++ = sdmmc_read32(REG_SDFIFO32);
							}
						}
						else
						{
						#endif
							for(int i = 0; i<0x200; i+=2)
							{
								*dataPtr++ = sdmmc_read16(REG_SDFIFO);
							}
						#ifdef DATA32_SUPPORT
						}
						#endif
						size -= 0x200;
					}
				}

				sdmmc_mask16(REG_DATACTL32, 0x800, 0);
			}
		}
#ifdef DATA32_SUPPORT
		if(!(ctl32 & 0x200))
#else
		if((status1 & TMIO_STAT1_TXRQ))
#endif
		{
			if(writedata)
			{
				if(useBuf)
				{
					sdmmc_mask16(REG_SDSTATUS1, TMIO_STAT1_TXRQ, 0);
					//sdmmc_write16(REG_SDSTATUS1,~TMIO_STAT1_TXRQ);
					if(size > 0x1FF)
					{
						#ifdef DATA32_SUPPORT
						for(int i = 0; i<0x200; i+=4)
						{
							sdmmc_write32(REG_SDFIFO32,*dataPtr32++);
						}
						#else
						for(int i = 0; i<0x200; i+=2)
						{
							sdmmc_write16(REG_SDFIFO,*dataPtr++);
						}
						#endif
						size -= 0x200;
					}
				}

				sdmmc_mask16(REG_DATACTL32, 0x1000, 0);
			}
		}
		if(status1 & TMIO_MASK_GW)
		{
			ctx->error |= 4;
			break;
		}

		if(!(status1 & TMIO_STAT1_CMD_BUSY))
		{
			status0 = sdmmc_read16(REG_SDSTATUS0);
			if(sdmmc_read16(REG_SDSTATUS0) & TMIO_STAT0_CMDRESPEND)
			{
				ctx->error |= 0x1;
			}
			if(status0 & TMIO_STAT0_DATAEND)
			{
				ctx->error |= 0x2;
			}

			if((status0 & flags) == flags)
				break;
		}
	}
	ctx->stat0 = sdmmc_read16(REG_SDSTATUS0);
	ctx->stat1 = sdmmc_read16(REG_SDSTATUS1);
	sdmmc_write16(REG_SDSTATUS0,0);
	sdmmc_write16(REG_SDSTATUS1,0);

	if(getSDRESP != 0)
	{
		ctx->ret[0] = sdmmc_read16(REG_SDRESP0) | (sdmmc_read16(REG_SDRESP1) << 16);
		ctx->ret[1] = sdmmc_read16(REG_SDRESP2) | (sdmmc_read16(REG_SDRESP3) << 16);
		ctx->ret[2] = sdmmc_read16(REG_SDRESP4) | (sdmmc_read16(REG_SDRESP5) << 16);
		ctx->ret[3] = sdmmc_read16(REG_SDRESP6) | (sdmmc_read16(REG_SDRESP7) << 16);
	}
}

int NO_INLINE sdmmc_sdcard_writesectors(uint32_t sector_no, uint32_t numsectors, uint8_t *in)
{
	if(handleSD.isSDHC == 0) sector_no <<= 9;
	inittarget(&handleSD);
	sdmmc_write16(REG_SDSTOP,0x100);
#ifdef DATA32_SUPPORT
	sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
	sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif
	sdmmc_write16(REG_SDBLKCOUNT,numsectors);
	handleSD.data = in;
	handleSD.size = numsectors << 9;
	sdmmc_send_command(&handleSD,0x52C19,sector_no);
	return geterror(&handleSD);
}

int NO_INLINE sdmmc_sdcard_readsectors(uint32_t sector_no, uint32_t numsectors, uint8_t *out)
{
	if(handleSD.isSDHC == 0) sector_no <<= 9;
	inittarget(&handleSD);
	sdmmc_write16(REG_SDSTOP,0x100);
#ifdef DATA32_SUPPORT
	sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
	sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif
	sdmmc_write16(REG_SDBLKCOUNT,numsectors);
	handleSD.data = out;
	handleSD.size = numsectors << 9;
	sdmmc_send_command(&handleSD,0x33C12,sector_no);
	return geterror(&handleSD);
}



int NO_INLINE sdmmc_nand_readsectors(uint32_t sector_no, uint32_t numsectors, uint8_t *out)
{
	if(handleNAND.isSDHC == 0) sector_no <<= 9;
	inittarget(&handleNAND);
	sdmmc_write16(REG_SDSTOP,0x100);
#ifdef DATA32_SUPPORT
	sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
	sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif
	sdmmc_write16(REG_SDBLKCOUNT,numsectors);
	handleNAND.data = out;
	handleNAND.size = numsectors << 9;
	sdmmc_send_command(&handleNAND,0x33C12,sector_no);
	inittarget(&handleSD);
	return geterror(&handleNAND);
}

int NO_INLINE sdmmc_nand_writesectors(uint32_t sector_no, uint32_t numsectors, uint8_t *in) //experimental
{
	if(handleNAND.isSDHC == 0) sector_no <<= 9;
	inittarget(&handleNAND);
	sdmmc_write16(REG_SDSTOP,0x100);
#ifdef DATA32_SUPPORT
	sdmmc_write16(REG_SDBLKCOUNT32,numsectors);
	sdmmc_write16(REG_SDBLKLEN32,0x200);
#endif
	sdmmc_write16(REG_SDBLKCOUNT,numsectors);
	handleNAND.data = in;
	handleNAND.size = numsectors << 9;
	sdmmc_send_command(&handleNAND,0x52C19,sector_no);
	inittarget(&handleSD);
	return geterror(&handleNAND);
}

static uint32_t calcSDSize(uint8_t* csd, int type)
{
  uint32_t result=0;
  if(type == -1) type = csd[14] >> 6;
  switch(type)
  {
    case 0:
      {
        uint32_t block_len=csd[9]&0xf;
        block_len=1<<block_len;
        uint32_t mult=(csd[4]>>7)|((csd[5]&3)<<1);
        mult=1<<(mult+2);
        result=csd[8]&3;
        result=(result<<8)|csd[7];
        result=(result<<2)|(csd[6]>>6);
        result=(result+1)*mult*block_len/512;
      }
      break;
    case 1:
      result=csd[7]&0x3f;
      result=(result<<8)|csd[6];
      result=(result<<8)|csd[5];
      result=(result+1)*1024;
      break;
  }
  return result;
}

void InitSD()
{
	//NAND
	handleNAND.isSDHC = 0;
	handleNAND.SDOPT = 0;
	handleNAND.res = 0;
	handleNAND.initarg = 1;
	handleNAND.clk = 0x80;
	handleNAND.devicenumber = 1;

	//SD
	handleSD.isSDHC = 0;
	handleSD.SDOPT = 0;
	handleSD.res = 0;
	handleSD.initarg = 0;
	handleSD.clk = 0x80;
	handleSD.devicenumber = 0;

	//sdmmc_mask16(0x100,0x800,0);
	//sdmmc_mask16(0x100,0x1000,0);
	//sdmmc_mask16(0x100,0x0,0x402);
	//sdmmc_mask16(0xD8,0x22,0x2);
	//sdmmc_mask16(0x100,0x2,0);
	//sdmmc_mask16(0xD8,0x22,0);
	//sdmmc_write16(0x104,0);
	//sdmmc_write16(0x108,1);
	//sdmmc_mask16(REG_SDRESET,1,0); //not in new Version -- nintendo's code does this
	//sdmmc_mask16(REG_SDRESET,0,1); //not in new Version -- nintendo's code does this
	//sdmmc_mask16(0x20,0,0x31D);
	//sdmmc_mask16(0x22,0,0x837F);
	//sdmmc_mask16(0xFC,0,0xDB);
	//sdmmc_mask16(0xFE,0,0xDB);
	////sdmmc_write16(REG_SDCLKCTL,0x20);
	////sdmmc_write16(REG_SDOPT,0x40EE);
	////sdmmc_mask16(0x02,0x3,0);
	//sdmmc_write16(REG_SDCLKCTL,0x40);
	//sdmmc_write16(REG_SDOPT,0x40EB);
	//sdmmc_mask16(0x02,0x3,0);
	//sdmmc_write16(REG_SDBLKLEN,0x200);
	//sdmmc_write16(REG_SDSTOP,0);

	*(volatile uint16_t*)0x10006100 &= 0xF7FFu; //SDDATACTL32
	*(volatile uint16_t*)0x10006100 &= 0xEFFFu; //SDDATACTL32
#ifdef DATA32_SUPPORT
	*(volatile uint16_t*)0x10006100 |= 0x402u; //SDDATACTL32
#else
	*(volatile uint16_t*)0x10006100 |= 0x402u; //SDDATACTL32
#endif
	*(volatile uint16_t*)0x100060D8 = (*(volatile uint16_t*)0x100060D8 & 0xFFDD) | 2;
#ifdef DATA32_SUPPORT
	*(volatile uint16_t*)0x10006100 &= 0xFFFFu; //SDDATACTL32
	*(volatile uint16_t*)0x100060D8 &= 0xFFDFu; //SDDATACTL
	*(volatile uint16_t*)0x10006104 = 512; //SDBLKLEN32
#else
	*(volatile uint16_t*)0x10006100 &= 0xFFFDu; //SDDATACTL32
	*(volatile uint16_t*)0x100060D8 &= 0xFFDDu; //SDDATACTL
	*(volatile uint16_t*)0x10006104 = 0; //SDBLKLEN32
#endif
	*(volatile uint16_t*)0x10006108 = 1; //SDBLKCOUNT32
	*(volatile uint16_t*)0x100060E0 &= 0xFFFEu; //SDRESET
	*(volatile uint16_t*)0x100060E0 |= 1u; //SDRESET
	*(volatile uint16_t*)0x10006020 |= TMIO_MASK_ALL; //SDIR_MASK0
	*(volatile uint16_t*)0x10006022 |= TMIO_MASK_ALL>>16; //SDIR_MASK1
	*(volatile uint16_t*)0x100060FC |= 0xDBu; //SDCTL_RESERVED7
	*(volatile uint16_t*)0x100060FE |= 0xDBu; //SDCTL_RESERVED8
	*(volatile uint16_t*)0x10006002 &= 0xFFFCu; //SDPORTSEL
#ifdef DATA32_SUPPORT
	*(volatile uint16_t*)0x10006024 = 0x20;
	*(volatile uint16_t*)0x10006028 = 0x40EE;
#else
	*(volatile uint16_t*)0x10006024 = 0x40; //Nintendo sets this to 0x20
	*(volatile uint16_t*)0x10006028 = 0x40EB; //Nintendo sets this to 0x40EE
#endif
	*(volatile uint16_t*)0x10006002 &= 0xFFFCu; ////SDPORTSEL
	*(volatile uint16_t*)0x10006026 = 512; //SDBLKLEN
	*(volatile uint16_t*)0x10006008 = 0; //SDSTOP

	inittarget(&handleSD);
}

int Nand_Init()
{
	inittarget(&handleNAND);
	waitcycles(0xF000);

	DEBUGPRINT(topScreen, "0x00000 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0,0);

	DEBUGPRINT(topScreen, "0x10701 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	do
	{
		do
		{
			sdmmc_send_command(&handleNAND,0x10701,0x100000);
			DEBUGPRINT(topScreen, "error ", handleNAND.error, 10, 20 + 17*8, RGB(40, 40, 40), RGB(208, 208, 208));
			DEBUGPRINT(topScreen, "ret: ", handleNAND.ret[0], 10, 20 + 18*8, RGB(40, 40, 40), RGB(208, 208, 208));
			DEBUGPRINT(topScreen, "test ", 3, 10, 20 + 19*8, RGB(40, 40, 40), RGB(208, 208, 208));
		} while ( !(handleNAND.error & 1) );
	}
	while((handleNAND.ret[0] & 0x80000000) == 0);

	DEBUGPRINT(topScreen, "0x10602 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x10602,0x0);
	if((handleNAND.error & 0x4))return -1;

	DEBUGPRINT(topScreen, "0x10403 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x10403,handleNAND.initarg << 0x10);
	if((handleNAND.error & 0x4))return -1;

	DEBUGPRINT(topScreen, "0x10609 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x10609,handleNAND.initarg << 0x10);
	if((handleNAND.error & 0x4))return -1;

	handleNAND.total_size = calcSDSize((uint8_t*)&handleNAND.ret[0],0);
	handleNAND.clk = 1;
	setckl(1);

	DEBUGPRINT(topScreen, "0x10407 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x10407,handleNAND.initarg << 0x10);
	if((handleNAND.error & 0x4))return -1;

	DEBUGPRINT(topScreen, "0x10506 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	handleNAND.SDOPT = 1;

	sdmmc_send_command(&handleNAND,0x10506,0x3B70100);
	if((handleNAND.error & 0x4))return -1;

	DEBUGPRINT(topScreen, "0x10506 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x10506,0x3B90100);
	if((handleNAND.error & 0x4))return -1;

	DEBUGPRINT(topScreen, "0x1040D ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x1040D,handleNAND.initarg << 0x10);
	if((handleNAND.error & 0x4))return -1;

	DEBUGPRINT(topScreen, "0x10410 ", handleNAND.error, 10, 20 + 13*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleNAND,0x10410,0x200);
	if((handleNAND.error & 0x4))return -1;

	handleNAND.clk |= 0x200;

	inittarget(&handleSD);

	return 0;
}

int SD_Init()
{
	inittarget(&handleSD);
	//waitcycles(0x3E8);
	waitcycles(0xF000);
	DEBUGPRINT(topScreen, "0x00000 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));
	sdmmc_send_command(&handleSD,0,0);
	DEBUGPRINT(topScreen, "0x10408 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));
	sdmmc_send_command(&handleSD,0x10408,0x1AA);
	//uint32_t temp = (handleSD.ret[0] == 0x1AA) << 0x1E;
	uint32_t temp = (handleSD.error & 0x1) << 0x1E;

	DEBUGPRINT(topScreen, "0x10769 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));
	DEBUGPRINT(topScreen, "sd ret: ", handleSD.ret[0], 10, 20 + 15*8, RGB(40, 40, 40), RGB(208, 208, 208));
	DEBUGPRINT(topScreen, "temp: ", temp, 10, 20 + 16*8, RGB(40, 40, 40), RGB(208, 208, 208));

	//int count = 0;
	uint32_t temp2 = 0;
	do
	{
		do
		{
			sdmmc_send_command(&handleSD,0x10437,handleSD.initarg << 0x10);
			sdmmc_send_command(&handleSD,0x10769,0x00FF8000 | temp);
			temp2 = 1;
		} while ( !(handleSD.error & 1) );

		//DEBUGPRINT(topScreen, "sd error ", handleSD.error, 10, 20 + 17*8, RGB(40, 40, 40), RGB(208, 208, 208));
		//DEBUGPRINT(topScreen, "sd ret: ", handleSD.ret[0], 10, 20 + 18*8, RGB(40, 40, 40), RGB(208, 208, 208));
		//DEBUGPRINT(topScreen, "count: ", count++, 10, 20 + 19*8, RGB(40, 40, 40), RGB(208, 208, 208));
	}
	while((handleSD.ret[0] & 0x80000000) == 0);
	//do
	//{
	//	sdmmc_send_command(&handleSD,0x10437,handleSD.initarg << 0x10);
	//	sdmmc_send_command(&handleSD,0x10769,0x00FF8000 | temp);
	//
	//	DEBUGPRINT(topScreen, "sd error ", handleSD.error, 10, 20 + 17*8, RGB(40, 40, 40), RGB(208, 208, 208));
	//	DEBUGPRINT(topScreen, "sd ret: ", handleSD.ret[0], 10, 20 + 18*8, RGB(40, 40, 40), RGB(208, 208, 208));
	//	DEBUGPRINT(topScreen, "count: ", count++, 10, 20 + 19*8, RGB(40, 40, 40), RGB(208, 208, 208));
	//}
	//while(!(handleSD.ret[0] & 0x80000000));

	if(!((handleSD.ret[0] >> 30) & 1) || !temp)
		temp2 = 0;

	handleSD.isSDHC = temp2;
	//handleSD.isSDHC = (handleSD.ret[0] & 0x40000000);

	DEBUGPRINT(topScreen, "0x10602 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x10602,0);
	if((handleSD.error & 0x4)) return -1;

	DEBUGPRINT(topScreen, "0x10403 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x10403,0);
	if((handleSD.error & 0x4)) return -1;
	handleSD.initarg = handleSD.ret[0] >> 0x10;

	DEBUGPRINT(topScreen, "0x10609 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x10609,handleSD.initarg << 0x10);
	if((handleSD.error & 0x4)) return -1;

	handleSD.total_size = calcSDSize((uint8_t*)&handleSD.ret[0],-1);
	handleSD.clk = 1;
	setckl(1);

	DEBUGPRINT(topScreen, "0x10507 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x10507,handleSD.initarg << 0x10);
	if((handleSD.error & 0x4)) return -1;

	DEBUGPRINT(topScreen, "0x10437 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x10437,handleSD.initarg << 0x10);
	if((handleSD.error & 0x4)) return -1;

	DEBUGPRINT(topScreen, "0x10446 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	handleSD.SDOPT = 1;
	sdmmc_send_command(&handleSD,0x10446,0x2);
	if((handleSD.error & 0x4)) return -1;

	DEBUGPRINT(topScreen, "0x1040D ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x1040D,handleSD.initarg << 0x10);
	if((handleSD.error & 0x4)) return -1;

	DEBUGPRINT(topScreen, "0x10410 ", handleSD.error, 10, 20 + 14*8, RGB(40, 40, 40), RGB(208, 208, 208));

	sdmmc_send_command(&handleSD,0x10410,0x200);
	if((handleSD.error & 0x4)) return -1;
	handleSD.clk |= 0x200;

	return 0;
}

void sdmmc_sdcard_init()
{
	DEBUGPRINT(topScreen, "sdmmc_sdcard_init ", handleSD.error, 10, 20 + 2*8, RGB(40, 40, 40), RGB(208, 208, 208));
	InitSD();
	//SD_Init2();
	//Nand_Init();
	uint32_t nand_res __attribute__((unused)) = Nand_Init();
	DEBUGPRINT(topScreen, "nand_res ", nand_res, 10, 20 + 3*8, RGB(40, 40, 40), RGB(208, 208, 208));
	uint32_t sd_res __attribute__((unused)) = SD_Init();
	DEBUGPRINT(topScreen, "sd_res ", sd_res, 10, 20 + 4*8, RGB(40, 40, 40), RGB(208, 208, 208));
}

int sdmmc_get_cid( int isNand, uint32_t *info)
{
	struct mmcdevice *device;
	if(isNand)
		device = &handleNAND;
	else
		device = &handleSD;

	inittarget(device);
	// use cmd7 to put sd card in standby mode
	// CMD7
	{
		sdmmc_send_command(device,0x10507,0);
		//if((device->error & 0x4)) return -1;
	}

	// get sd card info
	// use cmd10 to read CID
	{
		sdmmc_send_command(device,0x1060A,device->initarg << 0x10);
		//if((device->error & 0x4)) return -2;

		for( int i = 0; i < 4; ++i ) {
			info[i] = device->ret[i];
		}
	}

	// put sd card back to transfer mode
	// CMD7
	{
		sdmmc_send_command(device,0x10507,device->initarg << 0x10);
		//if((device->error & 0x4)) return -3;
	}

	if(isNand)
	{
		inittarget(&handleSD);
	}

	return 0;
}
