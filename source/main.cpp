#include "common.h"
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <malloc.h>

#include <elm.h>

#include "menu.h"
#include "ui.h"

#include "device.h"
#include "ntrcard.h"

#include "i2c.h"
#include "hid.h"
#include "crypto.h"

#include "blowfish_dev_bin.h"
#include "blowfish_retail_bin.h"

using flashcart_core::Flashcart;
using flashcart_core::flashcart_list;
namespace ntrcard = flashcart_core::ntrcard;

void ntrboot_flasher();

void ntrboot_dump_flash();
void ntrboot_restore_flash();
void ntrboot_inject();

Flashcart* selected_flashcart;
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
        i2cPoweroff();
    *(volatile u32*)0x10000020 = 0; // InitFS stuff
    *(volatile u32*)0x10000020 = 0x200; // InitFS stuff

    uint8_t **fb = (uint8_t **)(void *)argv[1];
    top_screen = fb[0];
    bottom_screen = fb[2];

    ntrboot_flasher();
    i2cPoweroff();

    return 0;
}

void ntrboot_flasher()
{
    if (!menu_show_intro_warning())
        return;

    ELM_Mount();

    menu_wait_cart_insert();

    reselect_cart: while(selected_flashcart == 0)
    {
        int8_t flashcart_index = menu_select_flashcart();
        if (flashcart_index == -1) {
            ELM_Unmount();
            return;
        }
        selected_flashcart = flashcart_list->at(flashcart_index);

        ntrcard::init();
        if (selected_flashcart->initialize())
            break;
        selected_flashcart->shutdown();
        selected_flashcart = 0;

        ShowPrompt(BOTTOM_SCREEN, false, "Your flashcart is not supported.");
    }

    while(true)
    {
        uint8_t menu_option = menu_flashcart_menu(selected_flashcart->getName());
        switch(menu_option)
        {
            case MENU_DUMP:
                ntrboot_dump_flash();
                break;
            case MENU_RESTORE:
                ntrboot_restore_flash();
                break;
            case MENU_INJECT:
                ntrboot_inject();
                break;
            case MENU_SELECT_FLASHCART:
                selected_flashcart->shutdown();
                selected_flashcart = 0;
                goto reselect_cart; // keeps code complexity down ¯\_(ツ)_/¯
                break;
            case MENU_EXIT:
                ELM_Unmount();
                return;
            case MENU_REBOOT:
                ELM_Unmount();
                i2cReboot();
                return;
        }
    }
}

void ntrboot_dump_flash() {
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);
    DrawString(TOP_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Dumping flash");

    uint32_t length = selected_flashcart->getMaxLength();

    uint8_t* mem = (uint8_t*)malloc(length);
    if (mem == NULL) {
        ShowPrompt(BOTTOM_SCREEN, false, "Failed to allocate memory");
        return;
    }

    //Create folder if it doesn't exist
    struct stat st;
    if (stat("fat1:/ntrboot", &st) == -1) {
        mkdir("fat1:/ntrboot", 0700);
    }

    FILE *f = fopen("fat1:/ntrboot/backup.bin","wb");
    if (f) {
        if (selected_flashcart->readFlash(0, length, mem)) {
            FILE *f = fopen("fat1:/ntrboot/backup.bin","wb");
            fwrite (mem, 1, length, f);
            fclose(f);

            DrawString(TOP_SCREEN, 10, 150, COLOR_GREEN, STD_COLOR_BG, "Dump Complete!");
        } else {
            ShowPrompt(BOTTOM_SCREEN, false, "Read from flash failed.");
        }
    } else {
        DrawStringF(TOP_SCREEN, 10, 30, COLOR_RED, STD_COLOR_BG,
            "Failed to open /ntrboot/backup.bin for writing\n"
            "(%d %s)", errno, strerror(errno));
    }

    free(mem);

    DrawString(TOP_SCREEN, 10, 160, STD_COLOR_FONT, STD_COLOR_BG, "Press <A> to return to the main menu.");

    WaitButton(BUTTON_A);
}

void ntrboot_restore_flash() {
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);
    DrawString(TOP_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Restoring flash");

    FILE *f = fopen("fat1:/ntrboot/backup.bin","rb");

    if(f != NULL)
    {
        fseek(f,0,SEEK_END);

        uint32_t flash_length = selected_flashcart->getMaxLength();
        uint8_t* flash_mem = (uint8_t*)memalign(4,flash_length);
        uint8_t* flash_memp = flash_mem;

        uint32_t length = ftell(f);
        if (length > flash_length) {
            length = flash_length;
        }
        uint8_t* mem = (uint8_t*)memalign(4,length);
        uint8_t* memp = mem;
        if (flash_mem && mem) {

            fseek(f,0,SEEK_SET);
            fread (mem, 1, length, f);

            DrawString(TOP_SCREEN, 10, 40, STD_COLOR_FONT, STD_COLOR_BG, "Reading...");
            selected_flashcart->readFlash(0, length, flash_memp);

            bool success = false;
            int written_chunk = 0;
            const int chunk_size = 64*1024;
            for(uint32_t j=0; j<length; j+=chunk_size)
            {
                DrawStringF(TOP_SCREEN, 10, 50, STD_COLOR_FONT, STD_COLOR_BG, "Checking %08X", j);
                if(memcmp(flash_memp+j,memp+j,chunk_size) != 0)
                {
                    DrawStringF(TOP_SCREEN, 10, 60, STD_COLOR_FONT, STD_COLOR_BG, "Writing chunk %08X", j);
                    success = selected_flashcart->writeFlash(j, chunk_size, memp+j);
                    if (!success) {
                        break;
                    }
                    written_chunk++;
                    DrawStringF(TOP_SCREEN, 10, 70, STD_COLOR_FONT, STD_COLOR_BG, "Chunks written %d (%08X)", written_chunk, written_chunk);
                }
            }

            if (success) {
                DrawString(TOP_SCREEN, 10, 90, COLOR_GREEN, STD_COLOR_BG, "Restoring Complete!");
            } else {
                DrawString(TOP_SCREEN, 10, 90, COLOR_GREEN, STD_COLOR_BG, "Restoring failed.");
            }
        } else {
            ShowPrompt(BOTTOM_SCREEN, false, "Failed to allocate memory");
        }

        if (flash_mem) {
            free(flash_mem);
        }
        if (mem) {
            free(mem);
        }

        fclose(f);
    }
    else
        DrawString(TOP_SCREEN, 10, 40, COLOR_RED, STD_COLOR_BG, "Restore file was not found.");

    DrawString(TOP_SCREEN, 10, 160, STD_COLOR_FONT, STD_COLOR_BG, "Press <A> to return to the main menu.");
    WaitButton(BUTTON_A);
}

void ntrboot_inject() {
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);
    DrawString(TOP_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG,
        "Injecting Ntrboot\n"
        "\n"
        "Press <A> for retail unit ntrboot\n"
        "Press <Y> for developer unit ntrboot\n"
        "Press <B> to return to the main menu.");

    FILE *f = NULL;
    FILE *fhash = NULL;
    uint8_t *blowfish_key = NULL;

    uint32_t button = WaitButton(BUTTON_A | BUTTON_B | BUTTON_Y);
    const char *fpath, *fhashpath;
    if (button & BUTTON_A)
    {
        fpath = "fat1:/ntrboot/boot9strap_ntr.firm";
        fhashpath = "fat1:/ntrboot/boot9strap_ntr.firm.sha";
        blowfish_key = (uint8_t*)blowfish_retail_bin;
    }
    else if (button & BUTTON_Y)
    {
        fpath = "fat1:/ntrboot/boot9strap_ntr_dev.firm";
        fhashpath = "fat1:/ntrboot/boot9strap_ntr_dev.firm.sha";
        blowfish_key = (uint8_t *)blowfish_dev_bin;
    }
    else if (button & BUTTON_B) return;
    else __builtin_unreachable();

    f = fopen(fpath, "rb");
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        size_t firm_size = ftell(f);
        uint8_t *firm = (uint8_t*)malloc(firm_size);

        if (firm) {
            fseek(f, 0, SEEK_SET);
            fread(firm, 1, firm_size, f);

            fhash = fopen(fhashpath, "rb");
            if (fhash) {
                uint8_t hash[32];
                fseek(fhash, 0, SEEK_SET);
                if (fread(hash, 1, sizeof(hash), fhash) == sizeof(hash)) {
                    if (crypto::sha256verify(firm, firm_size, hash)) {
                        DrawString(TOP_SCREEN, 10, 80, COLOR_GREEN, STD_COLOR_BG, "Injecting...");

                        if (selected_flashcart->injectNtrBoot((uint8_t *)blowfish_key, firm, firm_size)) {
                            DrawString(TOP_SCREEN, 10, 90, COLOR_GREEN, STD_COLOR_BG, "Injection Complete!");
                        } else {
                            DrawString(TOP_SCREEN, 10, 90, COLOR_RED, STD_COLOR_BG, "Injection failed.");
                        }
                    } else {
                        DrawString(TOP_SCREEN, 10, 80, COLOR_RED, STD_COLOR_BG, "Failed to verify FIRM integrity");
                    }
                } else {
                    DrawStringF(TOP_SCREEN, 10, 80, COLOR_RED, STD_COLOR_BG, "%s is corrupt", fhashpath + 5 /* "fat1:" */);
                }

                fclose(fhash);
            } else {
                DrawString(TOP_SCREEN, 10, 80, COLOR_RED, STD_COLOR_BG, "%s not found", fhashpath + 5 /* "fat1:" */);
            }

            free(firm);
        } else {
            DrawStringF(TOP_SCREEN, 10, 80, COLOR_RED, STD_COLOR_BG, "Failed to allocate memory");
        }

        fclose(f);
    } else
        DrawStringF(TOP_SCREEN, 10, 80, COLOR_RED, STD_COLOR_BG, "%s not found", fpath + 5 /* "fat1:" */);

    DrawString(TOP_SCREEN, 10, 100, STD_COLOR_FONT, STD_COLOR_BG, "Press <A> to return to the main menu.");
    WaitButton(BUTTON_A);
}
