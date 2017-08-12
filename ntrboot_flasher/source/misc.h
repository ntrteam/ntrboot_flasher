#pragma once

#include <cstddef>
#include <inttypes.h>
#include "3dstypes.h"

#define READ 1
#define WRITE 2
#define CREATE 4

#define REG_SYSPROT9 				(*(volatile uint8_t*)0x10000000)
#define REG_SYSPROT11 				(*(volatile uint8_t*)0x10000001)
#define REG_SYSPROT1 				(*(volatile uint8_t*)0x10000002)
#define REG_SYSPROT2 				(*(volatile uint8_t*)0x10000003)
#define REG_MPCORECFG 				(*(volatile uint32_t*)0x10000FFC)

#define KEYS 						(*(volatile uint32_t*)0x10146000)
#define PXICNT 						(*(volatile uint32_t*)0x10163004)
#define IE 							(*(volatile uint32_t*)0x10001000)
#define IF 							(*(volatile uint32_t*)0x10001004)

#define TM0VAL 						(*(volatile uint16_t*)0x10003000)
#define TM0CNT 						(*(volatile uint16_t*)0x10003002)
#define TM1VAL 						(*(volatile uint16_t*)0x10003004)
#define TM1CNT 						(*(volatile uint16_t*)0x10003006)
#define TM2VAL 						(*(volatile uint16_t*)0x10003008)
#define TM2CNT 						(*(volatile uint16_t*)0x1000300A)
#define TM3VAL 						(*(volatile uint16_t*)0x1000300C)
#define TM3CNT 						(*(volatile uint16_t*)0x1000300E)

#define NDMAGCNT 					(*(volatile uint32_t*)0x10002000)
#define NDMACNT(n) 					(*(volatile uint32_t*)(0x1000201C+n*0x1c))

#define REG_AESCNT 					(*(volatile uint32_t*)0x10009000)
#define REG_AESBLKCNT 				(*(volatile uint32_t*)0x10009004)
#define REG_AESBLKCNTH1 			(*(volatile uint16_t*)0x10009004)
#define REG_AESBLKCNTH2 			(*(volatile uint16_t*)0x10009006)
#define REG_AESWRFIFO 				(*(volatile uint32_t*)0x10009008)
#define REG_AESRDFIFO 				(*(volatile uint32_t*)0x1000900C)
#define REG_AESKEYSEL 				(*(volatile uint8_t*)0x10009010)
#define REG_AESKEYCNT 				(*(volatile uint8_t*)0x10009011)
#define REG_AESCTR 					((volatile uint32_t*)0x10009020) //16
#define REG_AESMAC 					((volatile uint32_t*)0x10009030) //16
#define REG_AESKEY0 				((volatile uint32_t*)0x10009040) //48
#define REG_AESKEY1 				((volatile uint32_t*)0x10009070) //48
#define REG_AESKEY2 				((volatile uint32_t*)0x100090A0) //48
#define REG_AESKEY3 				((volatile uint32_t*)0x100090D0) //48
#define REG_AESKEYFIFO 				(*(volatile uint32_t*)0x10009100)
#define REG_AESKEYXFIFO 			(*(volatile uint32_t*)0x10009104)
#define REG_AESKEYYFIFO 			(*(volatile uint32_t*)0x10009108)

#define REG_SHACNT 					(*(volatile uint32_t*)0x1000A000)
#define REG_SHABLKCNT 				(*(volatile uint32_t*)0x1000A004)
#define REG_SHAHASH 				((volatile uint32_t*)0x1000A040) //0x20
#define REG_SHAINFIFO 				(*(volatile uint32_t*)0x1000A080) //0x40

#define SHA_HASH_READY				(0<<0)
#define SHA_NORMAL					(1<<0)
#define SHA_FINAL_ROUND				(2<<0)
#define SHA_OUTPUT_LITTLE_ENDIAN	(0<<3)
#define SHA_OUTPUT_BIG_ENDIAN		(1<<3)
#define SHA_MODE_SHA256				(0<<4)
#define SHA_MODE_SHA224				(1<<4)
#define SHA_MODE_SHA1				(2<<4)
#define SHA_MODE_SHA1_				(3<<4)
#define SHA_ENABLE					(1<<16)
#define SHA_READY					(1<<17)

#define AES_WRITE_FIFO_COUNT		((REG_AESCNT>>0) & 0x1F)
#define AES_READ_FIFO_COUNT			((REG_AESCNT>>5) & 0x1F)
#define AES_BUSY					(1<<31)

#define AES_FLUSH_READ_FIFO			(1<<10)
#define AES_FLUSH_WRITE_FIFO		(1<<11)
#define AES_BIT12					(1<<12)
#define AES_BIT13					(1<<13)
#define AES_MAC_SIZE(n)				((n&7)<<16)
#define AES_MAC_REGISTER_SOURCE		(1<<20)
#define AES_UNKNOWN_21				(1<<21)
#define AES_MAC_STATUS				(1<<21) // AES_UNKNOWN_21
#define AES_OUTPUT_BIG_ENDIAN		(1<<22)
#define AES_INPUT_BIG_ENDIAN		(1<<23)
#define AES_OUTPUT_NORMAL_ORDER		(1<<24)
#define AES_INPUT_NORMAL_ORDER		(1<<25)
#define AES_UNKNOWN_26				(1<<26)
#define AES_UPDATE_KEYSLOT			(1<<26) // AES_UNKNOWN_26
#define AES_MODE(n)					((n&7)<<27)
#define AES_INTERRUPT_ENABLE		(1<<30)
#define AES_ENABLE					(1<<31)
#define AES_BLOCK_SIZE				0x10

#define AES_MODE_CCM_DECRYPT		(0)
#define AES_MODE_CCM_ENCRYPT		(1)
#define AES_MODE_CTR				(2)
#define AES_MODE_UNK3				(3)
#define AES_MODE_CBC_DECRYPT		(4)
#define AES_MODE_CBC_ENCRYPT		(5)
#define AES_MODE_ECB_DECRYPT		(6)
#define AES_MODE_ECB_ENCRYPT		(7)

#define AES_MODE_CTR2             	(2)
#define AES_MODE_CTR3             	(3)
#define AES_MODE_CTR6             	(6)
#define AES_MODE_CTR7             	(7)

#define AES_CRYPT_SET_NORMAL_KEY  	(1)
#define AES_CRYPT_SET_TWL_KEY     	(1<<1)
#define AES_CRYPT_SET_KEYY1       	(1<<2)
#define AES_CRYPT_SET_KEYY2       	(1<<3)
#define AES_CRYPT_SET_KEYX1       	(1<<4)
#define AES_CRYPT_SET_KEYX2       	(1<<5)
#define AES_CRYPT_SELECT_KEYSLOT  	(1<<6)

#define KEY_A 						(1<<0)
#define KEY_B 						(1<<1)
#define KEY_SELECT 					(1<<2)
#define KEY_START 					(1<<3)
#define KEY_RIGHT 					(1<<4)
#define KEY_LEFT 					(1<<5)
#define KEY_UP 						(1<<6)
#define KEY_DOWN 					(1<<7)
#define KEY_R 						(1<<8)
#define KEY_L 						(1<<9)
#define KEY_X 						(1<<10)
#define KEY_Y 						(1<<11)

#define BSWAP32(x)  ((((x) & 0x000000ff) << 24) |      \
                      (((x) & 0x0000ff00) << 8)  |      \
                      (((x) & 0x00ff0000) >> 8)  |      \
                      (((x) & 0xff000000) >> 24));

#define MRC(reg, processor, op1, crn, crm, op2) \
__asm volatile ( \
" .arm\n" \
"   mrc   "   #processor "," #op1 ", %0,"  #crn "," #crm "," #op2 "\n" \
: "=r" (reg))

#define MCR(reg, processor, op1, crn, crm, op2) \
__asm volatile ( \
" .arm\n" \
"   mcr   "   #processor "," #op1 ", %0,"  #crn "," #crm "," #op2 "\n" \
: : "r" (reg))

#define WAIT_FOR_A_PRESS() \
	while(1)\
	{		\
		uint32_t pad = KEYS;\
		if(!(pad & KEY_A))\
		{\
			break;\
		}\
	}
	
#define WAIT_FOR_B_PRESS() \
	while(1)\
	{		\
		uint32_t pad = KEYS;\
		if(!(pad & KEY_B))\
		{\
			break;\
		}\
	}

/*
* Easy access convenience function to read CP15 registers from c code
*/
#define MRC15(reg, op1, crn, crm, op2) MRC(reg, p15, op1, crn, crm, op2)
#define MCR15(reg, op1, crn, crm, op2) MCR(reg, p15, op1, crn, crm, op2)


//PADGEN stuff

#define MAXENTRIES 512

struct sd_info_entry {
	uint8_t CTR[16];
	uint32_t size_mb;
	uint16_t filename[90];
} __attribute__((packed));

struct sd_info {
	uint32_t n_entries;
	struct sd_info_entry entries[MAXENTRIES];
} __attribute__((packed, aligned(16)));


struct ncch_info_entry {
	uint8_t  CTR[16];
	uint8_t  keyY[16];
	uint32_t size_mb;
	uint8_t  reserved[8];
	uint32_t uses7xCrypto;
	uint16_t  filename[56]; 
} __attribute__((packed));

struct ncch_info {
	uint32_t padding;
	uint32_t ncch_info_version;
	uint32_t n_entries;
	uint8_t  reserved[4];
	struct ncch_info_entry entries[MAXENTRIES];
} __attribute__((packed, aligned(16)));


struct pad_info {
	uint32_t keyslot;
	uint32_t setKeyY;
	uint8_t CTR[16];
	uint8_t  keyY[16];
	uint32_t size_mb;
	uint16_t filename[90];
} __attribute__((packed, aligned(16)));

struct title_key_entry {
	uint32_t commonKeyIndex;
	uint8_t  reserved[4];
	uint8_t  titleId[8];
	uint8_t  encryptedTitleKey[16];
} __attribute__((packed));

struct enckeys_info {
	uint32_t n_entries;
	uint8_t  reserved[12];
	struct title_key_entry entries[MAXENTRIES];
} __attribute__((packed, aligned(16)));

