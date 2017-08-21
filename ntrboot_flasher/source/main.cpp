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

int file_exist (const char *filename)
{
    struct stat   buffer;
    return (stat (filename, &buffer) == 0);
}

/*-----------------------------------------------------------------
main
This is the main for the code running on arm9
-----------------------------------------------------------------*/
int main(int argc, char** argv)
{
    if(argc < 2)
        handleExit();
    *(volatile u32*)0x10000020 = 0; // InitFS stuff
    *(volatile u32*)0x10000020 = 0x200; // InitFS stuff

    uint8_t **fb = (uint8_t **)(void *)argv[1];
    top_screen = fb[0];
    bottom_screen = fb[2];

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
        while(KEYS != 0); //Wait for key to be let go

        if(pad & KEY_DOWN)
        {
            menuOption++;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(pad & KEY_UP)
        {
            menuOption--;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(pad & KEY_RIGHT)
        {
            menuOption += 5;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(pad & KEY_LEFT)
        {
            menuOption -= 5;
            ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
        }

        if(pad & KEY_A)
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
    while(!i2cWriteRegister(I2C_DEV_MCU, 0x20, 1));
    while(true);
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
        struct stat st;
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
    while(!(KEYS & KEY_B));
}

void handleRestoreFlash()
{
    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Restoring flash");

    DrawStringF(TOP_SCREEN, 10, 40, "Press Y if you want to proceed.");
    DrawStringF(TOP_SCREEN, 10, 50, "Press B to return to the main menu.");
    while(true)
    {
        uint32_t pad = KEYS;
        if(pad & KEY_B)
            return;
        else if(pad & KEY_Y)
            break;
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Restoring flash");

    ELM_Mount();

    struct stat st;
    if (stat("fat1:/ntrboot", &st) == -1)
    {
        DrawStringF(TOP_SCREEN, 10, 130, "ntrboot folder not found on sd!");
        DrawStringF(TOP_SCREEN, 10, 140, "Please prepare this folder and try again");
        DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
        while(!(KEYS & KEY_B));
        return;
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
            for(uint32_t j=0; j<length; j+=chunk_size)
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
        DrawStringF(TOP_SCREEN, 10, 150, "Restore file was not found.");
    ELM_Unmount();

    ShowProgress(TOP_SCREEN, 0, 0);
    DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
    while(!(KEYS & KEY_B));
}

void handleInject()
{
    ELM_Mount();

    struct stat st;
    if (stat("fat1:/ntrboot", &st) == -1)
    {
        DrawStringF(TOP_SCREEN, 10, 130, "ntrboot folder not found on sd!");
        DrawStringF(TOP_SCREEN, 10, 140, "Please prepare this folder and try again");
        DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
        while(!(KEYS & KEY_B));
    }

    if(!file_exist("fat1:/ntrboot/backup.bin"))
    {
        DrawStringF(TOP_SCREEN, 10, 20, "No backup of flash detected.");
        DrawStringF(TOP_SCREEN, 10, 30, "Please dump your flash first then run this again.");
        DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
        while(!(KEYS & KEY_B));
        ELM_Unmount();
        return;
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Injecting Ntrboot");

    DrawStringF(TOP_SCREEN, 10, 40, "Press Y if you want to proceed.");
    DrawStringF(TOP_SCREEN, 10, 50, "Press B to return to the main menu.");
    while(true)
    {
        uint32_t pad = KEYS;
        if(pad & KEY_B)
        {
            ELM_Unmount();
            return;
        }
        else if(!(pad & KEY_Y))
            break;
    }

    ClearScreen(TOP_SCREEN, DBG_COLOR_BG);
    DrawStringF(TOP_SCREEN, 10, 20, "Injecting Ntrboot");

    DrawStringF(TOP_SCREEN, 10, 40, "Press L for developer unit ntrboot");
    DrawStringF(TOP_SCREEN, 10, 50, "Press R for retail unit ntrboot");
    DrawStringF(TOP_SCREEN, 10, 60, "Press B to return to the main menu.");

    FILE *f = NULL;
    uint8_t *blowfish_key = NULL;

    while(true)
    {
        uint32_t pad = KEYS;
        if(pad & KEY_L)
        {
            f = fopen("fat1:/ntrboot/boot9strap_ntr_dev.firm","rb");
            blowfish_key = (uint8_t *)blowfish_dev_bin;
            break;
        }
        if(pad & KEY_R)
        {
            f = fopen("fat1:/ntrboot/boot9strap_ntr.firm","rb");
            blowfish_key = (uint8_t *)blowfish_retail_bin;
            break;
        }
        if(pad & KEY_B)
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
        DrawStringF(TOP_SCREEN, 10, 150, "Firm was not found.");

    ELM_Unmount();

    DrawStringF(TOP_SCREEN, 10, 160, "Press B to return to the main menu.");
    while(!(KEYS & KEY_B));
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
