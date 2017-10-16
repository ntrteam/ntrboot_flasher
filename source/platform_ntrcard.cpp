#include <cstdint>
#include <cstring>

#include "types.h"
#include "ntrcard.h"
#include "device.h"
#include "platform.h"
#include "delay.h"

#include "blowfish_dev_bin.h"
#include "blowfish_retail_bin.h"

#define REG_CARDCONF            (*reinterpret_cast<volatile uint16_t *>(0x1000000C))
#define REG_CARDCONF2           (*reinterpret_cast<volatile uint8_t *>(0x10000010))

#define REG_CTRCARD_SECCNT      (*reinterpret_cast<volatile uint32_t *>(0x10004008))
#define REG_MCNT                (*reinterpret_cast<volatile uint16_t *>(0x10164000))
#define REG_MDATA               (*reinterpret_cast<volatile uint16_t *>(0x10164002))
#define REG_ROMCNT              (*reinterpret_cast<volatile uint32_t *>(0x10164004))
#define REG_CMDP                (reinterpret_cast<volatile uint8_t *>(0x10164008))
#define REG_CMD                 (*reinterpret_cast<volatile uint64_t *>(0x10164008))
#define REG_SEEDX_L             (*reinterpret_cast<volatile uint32_t *>(0x10164010))
#define REG_SEEDY_L             (*reinterpret_cast<volatile uint32_t *>(0x10164014))
#define REG_SEEDX_H             (*reinterpret_cast<volatile uint16_t *>(0x10164018))
#define REG_SEEDY_H             (*reinterpret_cast<volatile uint16_t *>(0x1016401A))
#define REG_FIFO                (*reinterpret_cast<volatile uint32_t *>(0x1016401C))

#define ROMCNT_ACTIVATE         (1u << 31)              // begin operation (CS low when set)
#define ROMCNT_BUSY             (ROMCNT_ACTIVATE)       // operation in progress i.e. CS still low
#define ROMCNT_WR               (1u << 30)              // card write enable
#define ROMCNT_NRESET           (1u << 29)              // RESET high when set
#define ROMCNT_SEC_LARGE        (1u << 28)              // Use "other" secure area mode, which tranfers blocks of 0x1000 bytes at a time
#define ROMCNT_CLK_SLOW         (1u << 27)              // Transfer clock rate (0 = 6.7MHz, 1 = 4.2MHz)
#define ROMCNT_BLK_SIZE(n)      (((n) & 0x7u) << 24)    // Transfer block size, (0 = None, 1..6 = (0x100 << n) bytes, 7 = 4 bytes)
#define ROMCNT_BLK_SIZE_MASK    (ROMCNT_BLK_SIZE(7))
#define ROMCNT_DATA_READY       (1u << 23)              // REG_FIFO is ready to be read
#define ROMCNT_SEC_CMD          (1u << 22)              // The command transfer will be hardware encrypted (KEY2)
#define ROMCNT_DELAY2(n)        (((n) & 0x3Fu) << 16)   // Transfer delay length part 2
#define ROMCNT_DELAY2_MASK      (ROMCNT_DELAY2(0x3F))
#define ROMCNT_SEC_SEED         (1u << 15)              // Apply encryption (KEY2) seed to hardware registers
#define ROMCNT_SEC_EN           (1u << 14)              // Security enable
#define ROMCNT_SEC_DAT          (1u << 13)              // The data transfer will be hardware encrypted (KEY2)
#define ROMCNT_DELAY1(n)        ((n) & 0x1FFFu)         // Transfer delay length part 1
#define ROMCNT_DELAY1_MASK      (ROMCNT_DELAY1(0x1FFF))
#define ROMCNT_CMD_SETTINGS     (ROMCNT_DELAY1_MASK | ROMCNT_DELAY2_MASK | ROMCNT_SEC_LARGE | \
                                    ROMCNT_SEC_CMD | ROMCNT_SEC_DAT | ROMCNT_CLK_SLOW | ROMCNT_SEC_EN)

#define MCNT_CR1_ENABLE         0x8000u
#define MCNT_CR1_IRQ            0x4000u

using std::uint8_t;
using std::uint16_t;
using std::int32_t;
using std::uint32_t;
using std::uint64_t;

bool isCardPresent() {
    return !(REG_CARDCONF2 & 0x1);
}

void waitForCard() {
    while (REG_CARDCONF2 & 0x1);
}

namespace flashcart_core {
namespace platform {
extern const bool HAS_HW_KEY2 = true;

int32_t resetCard() {
    if (REG_CARDCONF2 & 0x1) {
        return -1;
    }

    REG_CARDCONF2 = 0x0C;
    REG_CARDCONF &= ~3;
    if (REG_CARDCONF2 == 0xC) {
        while (REG_CARDCONF2 != 0);
    }
    if (REG_CARDCONF2 != 0) {
        return -2;
    }
    REG_CARDCONF2 = 0x4;
    while (REG_CARDCONF2 != 0x4);
    REG_CARDCONF2 = 0x8;
    while (REG_CARDCONF2 != 0x8);

    REG_CTRCARD_SECCNT &= 0xFFFFFFFB;
    ::ioDelay(0x40000);

    REG_ROMCNT = ROMCNT_NRESET;
    REG_CARDCONF &= ~3;
    REG_CARDCONF &= ~0x100;
    REG_MCNT = MCNT_CR1_ENABLE;
    ::ioDelay(0x40000);

    REG_ROMCNT = 0;
    REG_MCNT &= 0xFF;
    ::ioDelay(0x40000);

    REG_MCNT |= (MCNT_CR1_ENABLE | MCNT_CR1_IRQ);
    REG_ROMCNT = ROMCNT_NRESET | ROMCNT_SEC_SEED;
    while (REG_ROMCNT & ROMCNT_BUSY);

    return 0;
}

bool sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *const resp, ntrcard::OpFlags flags) {
    uint32_t blksizeflag;
    switch (response_len) {
        default: return false;
        case 0: blksizeflag = 0; break;
        case 4: blksizeflag = 7; break;
        case 0x200: blksizeflag = 1; break;
        case 0x400: blksizeflag = 2; break;
        case 0x800: blksizeflag = 3; break;
        case 0x1000: blksizeflag = 4; break;
        case 0x2000: blksizeflag = 5; break;
        case 0x4000: blksizeflag = 6; break;
    }
    REG_CMD = *reinterpret_cast<const uint64_t *>(cmdbuf);
    uint32_t bitflags = ROMCNT_ACTIVATE | ROMCNT_NRESET | ROMCNT_BLK_SIZE(blksizeflag) |
        ((flags.key2_command() || flags.key2_response()) ? ROMCNT_SEC_EN : 0) |
        (static_cast<uint32_t>(flags) & ROMCNT_CMD_SETTINGS);
    platform::logMessage(LOG_DEBUG, "ROMCNT = 0x%08X", bitflags);
    REG_ROMCNT = bitflags;

    uint32_t *cur = reinterpret_cast<uint32_t *>(resp);
    uint32_t ctr = 0;
    do {
        if (REG_ROMCNT & ROMCNT_DATA_READY) {
            uint32_t data = REG_FIFO;
            if (resp && ctr < response_len) {
                *(cur++) = data;
                ctr += 4;
            } else {
                (void)data;
            }
        }
	} while (REG_ROMCNT & ROMCNT_BUSY);
    return true;
}

void ioDelay(std::uint32_t us) {
    ::ioDelay(us);
}

void initBlowfishPS(std::uint32_t (&ps)[ntrcard::BLOWFISH_PS_N], ntrcard::BlowfishKey key) {
    const void *ptr;
    switch (key) {
        default: // blah
        case ntrcard::BlowfishKey::NTR:
            ptr = reinterpret_cast<void *>(0x01FFE428);
            break;
        case ntrcard::BlowfishKey::B9RETAIL:
            ptr = blowfish_retail_bin;
            break;
        case ntrcard::BlowfishKey::B9DEV:
            ptr = blowfish_dev_bin;
            break;
    }

    std::memcpy(ps, ptr, sizeof(ps));
    static_assert(sizeof(ps) == 0x1048, "Wrong Blowfish PS size");
}

void initKey2Seed(uint64_t x, uint64_t y) {
    REG_ROMCNT = 0;
    uint32_t xl = static_cast<uint32_t>(x & 0xFFFFFFFF);
    uint32_t yl = static_cast<uint32_t>(y & 0xFFFFFFFF);
    uint16_t xh = static_cast<uint16_t>((x >> 32) & 0x7F);
    uint16_t yh = static_cast<uint16_t>((y >> 32) & 0x7F);
    REG_SEEDX_L = xl;
    REG_SEEDY_L = yl;
    REG_SEEDX_H = xh;
    REG_SEEDY_H = yh;
    platform::logMessage(LOG_DEBUG, "Seeding KEY2: x = %02X %08X y = %02X %08X", xh, xl, yh, yl);
    REG_ROMCNT = ROMCNT_NRESET | ROMCNT_SEC_SEED | ROMCNT_SEC_EN | ROMCNT_SEC_DAT;
}
}
}
