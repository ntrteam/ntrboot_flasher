#include <cstdio>
#include <cstdlib>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <elm.h>
#include <sdmmc.h>
#include <dirent.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <sys/iosupport.h>
#include "3dstypes.h"

#include "gamecart/protocol.h"
#include "gamecart/protocol_ntr.h"
#include "flashcart_core/device.h"

#include "misc.h"
#include "DrawCharacter.h"

#include "i2c.h"
#include "delay.h"
#include "blowfish_dev_bin.h"
#include "blowfish_retail_bin.h"

//Uncomment this if you want to do a FULL restore... this will take much longer to write
//#define FULL_RESTORE

void handleMainMenu();
void handleDumpFlash();
void handleRestoreFlash();
void handleInject();
void handleExit();

uint8_t *top_screen, *bottom_screen;

static const uint16_t crc16tab[256]= {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

uint16_t crc16(uint8_t *buf, int len) {
    int counter;
    uint16_t crc = 0;
    for (counter = 0; counter < len; counter++)
            crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *buf++)&0x00FF];
    return crc;
}


FILE *log_file = NULL;

int file_exist (const char *filename)
{
    struct stat   buffer;
    return (stat (filename, &buffer) == 0);
}

//---------------------------------------------------------------------------------
ssize_t file_write(struct _reent *r, void *fd, const char *ptr, size_t len) {
//---------------------------------------------------------------------------------
    if(len > 1)
    {
        fprintf(log_file,"%.*s",len,ptr);
    }
    return len;
}

static const devoptab_t dotab_file = {
    "file",
    0,
    NULL,
    NULL,
    file_write,
    NULL,
    NULL,
    NULL
};

void OpenLog(char * filename)
{
    log_file = fopen(filename,"wb");
    devoptab_list[STD_OUT] = &dotab_file;
    int buffertype = _IOLBF;
    setvbuf(stdout, NULL , buffertype, 0);
}

void FlushLog()
{
    fclose(log_file);
}

void ResetDSP()
{
    *(uint8_t *)0x10140000 = -128;
    *(uint8_t *)0x10140001 = -124;
    *(uint8_t *)0x10140002 = -120;
    *(uint8_t *)0x10140003 = -116;
    *(uint8_t *)0x10140004 = -112;
    *(uint8_t *)0x10140005 = -108;
    *(uint8_t *)0x10140006 = -104;
    *(uint8_t *)0x10140007 = -100;
    *(uint8_t *)0x10140008 = -128;
    *(uint8_t *)0x10140009 = -124;
    *(uint8_t *)0x1014000A = -120;
    *(uint8_t *)0x1014000B = -116;
    *(uint8_t *)0x1014000C = -112;
    *(uint8_t *)0x1014000D = -108;
    *(uint8_t *)0x1014000E = -104;
    *(uint8_t *)0x1014000F = -100;
}

void ResetRegisters()
{
    *(uint8_t *)0x10141230 = 2;
    //Delay2(10);
    *(uint8_t *)0x10141230 = 3;
    //Delay2(10);

    ResetDSP();

    //Clear NDMA
    for (int i = 0; i <= 7; ++i )
        *(uint32_t *)((28 * i) + 0x1000201C) = (unsigned int)(2 * *(uint32_t *)((28 * i) + 0x1000201C)) >> 1;

    *(uint32_t *)0x1000C020 = 0;
    *(uint32_t *)0x1000C02C = -1;

    ClearScreen(TOP_SCREEN, RGB(0, 0, 0));
}

/*-----------------------------------------------------------------
main
This is the main for the code running on arm9
-----------------------------------------------------------------*/
int main(int argc, char** argv)
{
    *(volatile u32*)0x10000020 = 0; // InitFS stuff
    *(volatile u32*)0x10000020 = 0x200; // InitFS stuff

    // Fetch the framebuffer addresses
    if(argc >= 2) {
        // newer entrypoints
        uint8_t **fb = (uint8_t **)(void *)argv[1];
        top_screen = fb[0];
        bottom_screen = fb[2];
    } else {
        // outdated entrypoints
        top_screen = (uint8_t*)(*(u32*)0x23FFFE00);
        bottom_screen = (uint8_t*)(*(u32*)0x23FFFE08);
    }

    ResetRegisters();

    handleMainMenu();

    return 0;
}

enum
{
    MMOPTION_DUMP=0,
    MMOPTION_RESTORE,
    MMOPTION_INJECT,
    MMOPTION_EXIT,
    MMOPTION_MAX
};

const char *options[]={
    "Dump Flash",
    "Restore Flash",
    "Inject Ntrboot",
    "EXIT",
    "MAX"
};

void printMenuOptions(int menuOption)
{
    DrawStringF(TOP_SCREEN, 20, 20, "ntrboot flasher 0.1.2");

    for(int i=0; i<MMOPTION_MAX; i++)
        DrawStringF(TOP_SCREEN, 10, 30 + (2+i)*8, options[i]);

    //DrawHexWithName(TOP_SCREEN, "MenuOption = ", menuOption, 220, 1, RGB(0, 0, 0), bgcolor);
    DrawStringF(TOP_SCREEN, 1, (30 + ((2+menuOption)*8)), ">");
}

void handleMainMenu()
{
    int menuOption = 0;

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);

    while(true)
    {
        uint32_t pad = KEYS;
        printMenuOptions(menuOption);

        while(pad == KEYS);
        pad = KEYS;
        while(0xFFF != KEYS); //Wait for key to be let go

        if(!(pad & KEY_DOWN))
        {
            menuOption++;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(!(pad & KEY_UP))
        {
            menuOption--;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(!(pad & KEY_RIGHT))
        {
            menuOption+=5;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(!(pad & KEY_LEFT))
        {
            menuOption-=5;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(!(pad & KEY_A))
        {
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
            switch(menuOption)
            {
                case MMOPTION_DUMP:
                    handleDumpFlash();
                    break;
                case MMOPTION_RESTORE:
                    handleRestoreFlash();
                    break;
                case MMOPTION_INJECT:
                    handleInject();
                    break;
                case MMOPTION_EXIT:
                    handleExit();
                    break;
                default:
                    break;
            }
        ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    }

    if(menuOption < 0)
        menuOption = 0;
    else if(menuOption >= MMOPTION_MAX)
        menuOption = MMOPTION_MAX-1;
    }
}

void handleExit()
{
    while(!i2cWriteRegister(I2C_DEV_MCU, I2CREGPM_BATTERY, 1));
}

void handleDumpFlash()
{
    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Dumping flash");

    DrawStringF(TOP_SCREEN, 10, 30, "ChipID: %08X", Flashcart::getChipID());
    DrawStringF(TOP_SCREEN, 10, 40, "HW Rev: %08X", Flashcart::getHardwareVersion());

    ELM_Mount();

    Flashcart *cart = Flashcart::detectCart();
    if (cart) {
        DrawStringF(TOP_SCREEN, 10, 50, "Detected: %s", cart->getDescription());

        uint32_t length = cart->getMaxLength();
        uint8_t* mem = (uint8_t*)memalign(4,length);
        uint8_t* memp = mem;

        ShowProgress(TOP_SCREEN, 0, 0);
        cart->readFlash(0, length, memp);

        //Create folder if it doesn't exist
        struct stat st = {0};
        if (stat("fat1:/ntrboot", &st) == -1) {
            mkdir("fat1:/ntrboot", 0700);
        }

        FILE *f = fopen("fat1:/ntrboot/backup.bin","wb");
        fwrite (mem, 1, length, f);
        fclose(f);

        free(mem);
        ShowProgress(TOP_SCREEN, 0, 0);

        cart->cleanup();
        DrawStringF(TOP_SCREEN, 10, 150, "Dump Complete!");
    } else {
        DrawStringF(TOP_SCREEN, 10, 50, "Supported cartridge not detected!");
    }

    ELM_Unmount();

    DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
    while(1)
    {
        uint32_t pad = KEYS;
        if(!(pad & KEY_B))
        {
            return;
        }
    }
}

void handleRestoreFlash()
{
    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Restoring flash");

    DrawStringF(TOP_SCREEN, 10, 40, "Press Y if you want to proceed.");
    DrawStringF(TOP_SCREEN, 10, 50, "Press B to return to the main menu.");
    while(1)
    {
        uint32_t pad = KEYS;
        if(!(pad & KEY_B))
        {
            return;
        }
        if(!(pad & KEY_Y))
        {
            break;
        }
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Restoring flash");

    ELM_Mount();

    struct stat st = {0};
    if (stat("fat1:/ntrboot", &st) == -1)
    {
        DrawStringF(TOP_SCREEN, 10, 130, "ntrboot folder not found on sd!");
        DrawStringF(TOP_SCREEN, 10, 140, "Please prepare this folder and try again");
        DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
        while(1)
        {
            uint32_t pad = KEYS;
            if(!(pad & KEY_B))
            {
                return;
            }
        }
    }

    FILE *f = fopen("fat1:/ntrboot/backup.bin","rb");

    if(f != NULL)
    {
        fseek(f,0,SEEK_END);

        uint32_t length = ftell(f);
        uint8_t* mem = (uint8_t*)memalign(4,length);
        uint8_t* memp = mem;

        fseek(f,0,SEEK_SET);
        fread (mem, 1, length, f);

        fclose(f);

        DrawStringF(TOP_SCREEN, 10, 70, "Detecting Cart...");

        Flashcart *cart = Flashcart::detectCart();
        if (cart != nullptr) {
            DrawStringF(TOP_SCREEN, 10, 70, "Detected: %s", cart->getDescription());

#ifdef FULL_RESTORE
            DrawStringF(TOP_SCREEN, 10, 90, "Writing...");
            ShowProgress(TOP_SCREEN, 0, 0);
            cart->writeFlash(0, length, memp);
#else
            uint32_t flash_length = cart->getMaxLength();
            uint8_t* flash_mem = (uint8_t*)memalign(4,flash_length);
            uint8_t* flash_memp = flash_mem;

            DrawStringF(TOP_SCREEN, 10, 80, "Reading...");
            ShowProgress(TOP_SCREEN, 0, 0);
            cart->readFlash(0, length, flash_memp);

            int written_chunk = 0;
            const int chunk_size = 64*1024;
            for(int j=0; j<length; j+=chunk_size)
            {
                DrawStringF(TOP_SCREEN, 10, 90, "Checking %08X", j);
                ShowProgress(TOP_SCREEN, j, length);
                if(memcmp(flash_memp+j,memp+j,chunk_size) != 0)
                {
                    DrawStringF(TOP_SCREEN, 10, 100, "Writing chunk %08X", j);
                    cart->writeFlash(j, chunk_size, memp+j);
                    written_chunk++;
                    DrawStringF(TOP_SCREEN, 10, 110, "Chunks written %d (%08X)", written_chunk, written_chunk);
                }
            }

            free(flash_mem);
#endif

            cart->cleanup();
            DrawStringF(TOP_SCREEN, 10, 150, "Restoring Complete!");
        } else {
            DrawStringF(TOP_SCREEN, 10, 70, "Supported cartridge not detected!");
        }

        free(mem);
    }
    else
    {
        DrawStringF(TOP_SCREEN, 10, 150, "Restore file was not found.");
    }
    ELM_Unmount();

    ShowProgress(TOP_SCREEN, 0, 0);
    DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
    while(1)
    {
        uint32_t pad = KEYS;
        if(!(pad & KEY_B))
        {
            return;
        }
    }
}

void handleInject()
{
    ELM_Mount();

    struct stat st = {0};
    if (stat("fat1:/ntrboot", &st) == -1)
    {
        DrawStringF(TOP_SCREEN, 10, 130, "ntrboot folder not found on sd!");
        DrawStringF(TOP_SCREEN, 10, 140, "Please prepare this folder and try again");
        DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
        while(1)
        {
            uint32_t pad = KEYS;
            if(!(pad & KEY_B))
            {
                return;
            }
        }
    }

    if(!file_exist("fat1:/ntrboot/backup.bin"))
    {
        DrawStringF(TOP_SCREEN, 10, 20, "No backup of flash detected.");
        DrawStringF(TOP_SCREEN, 10, 30, "Please dump your flash first then run this again.");
        DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
        while(1)
        {
            uint32_t pad = KEYS;
            if(!(pad & KEY_B))
            {
                ELM_Unmount();
                return;
            }
        }
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Injecting Ntrboot");

    DrawStringF(TOP_SCREEN, 10, 40, "Press Y if you want to proceed.");
    DrawStringF(TOP_SCREEN, 10, 50, "Press B to return to the main menu.");
    while(1)
    {
        uint32_t pad = KEYS;
        if(!(pad & KEY_B))
        {
            ELM_Unmount();
            return;
        }
        if(!(pad & KEY_Y))
        {
            break;
        }
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Injecting Ntrboot");


    DrawStringF(TOP_SCREEN, 10, 40, "Press L for developer unit ntrboot");
    DrawStringF(TOP_SCREEN, 10, 50, "Press R for retail unit ntrboot");
    DrawStringF(TOP_SCREEN, 10, 60, "Press B to return to the main menu.");

    FILE *f = NULL;
    uint8_t *blowfish_key = NULL;

    while(1)
    {
        uint32_t pad = KEYS;
        if(!(pad & KEY_L))
        {
            f = fopen("fat1:/ntrboot/boot9strap_ntr_dev.firm","rb");
            blowfish_key = (uint8_t *)blowfish_dev_bin;
            break;
        }
        if(!(pad & KEY_R))
        {
            f = fopen("fat1:/ntrboot/boot9strap_ntr.firm","rb");
            blowfish_key = (uint8_t *)blowfish_retail_bin;
            break;
        }
        if(!(pad & KEY_B))
        {
            ELM_Unmount();
            return;
        }
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Injecting Ntrboot");

    if(f != NULL)
    {
        DrawStringF(TOP_SCREEN, 10, 70, "Detecting Cart...");
        fseek(f, 0, SEEK_END);

        size_t firm_size = ftell(f);
        uint8_t *firm = (uint8_t*)malloc(firm_size);

        fseek(f, 0, SEEK_SET);
        fread(firm, 1, firm_size, f);

        fclose(f);

        Flashcart *cart = Flashcart::detectCart();
        if (cart != nullptr) {
            DrawStringF(TOP_SCREEN, 10, 80, "Detected: %s", cart->getDescription());

            DrawStringF(TOP_SCREEN, 10, 90, "Injecting...");
            ShowProgress(TOP_SCREEN, 0, 0);
            cart->writeBlowfishAndFirm(blowfish_key, firm, firm_size);
            ShowProgress(TOP_SCREEN, 0, 0);

            cart->cleanup();
            DrawStringF(TOP_SCREEN, 10, 150, "Injection Complete!");
        } else {
            DrawStringF(TOP_SCREEN, 10, 80, "Supported cartridge not detected!");
        }

        free(firm);
    }
    else
    {
        DrawStringF(TOP_SCREEN, 10, 150, "Firm was not found.");
    }

    ELM_Unmount();

    DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
    while(1)
    {
        uint32_t pad = KEYS;
        if(!(pad & KEY_B))
        {
            return;
        }
    }
}

// Platform specific, should find a better way to handle these in the future.
// Works okay for now. >_>
void Flashcart::platformInit(){
    Cart_NTRInit();
}

void Flashcart::sendCommand(const uint8_t *cmdbuf, uint16_t response_len, uint8_t *resp, uint32_t flags) {
    NTR_SendCommand(cmdbuf, response_len, flags, resp);
}

void Flashcart::showProgress(uint32_t current, uint32_t total) {
    ShowProgress(TOP_SCREEN, current, total);
}
