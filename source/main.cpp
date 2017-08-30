// i don't think we need to include half this shit
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

#include "protocol.h"
#include "protocol_ntr.h"
#include "device.h"

#include "common.h"
#include "menu.h"
#include "ui.h"

#include "i2c.h"
#include "hid.h"
#include "delay.h"

#include "blowfish_dev_bin.h"
#include "blowfish_retail_bin.h"

void ntrboot_flasher();
void handle_exit();

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
        handle_exit();
    *(volatile u32*)0x10000020 = 0; // InitFS stuff
    *(volatile u32*)0x10000020 = 0x200; // InitFS stuff

    uint8_t **fb = (uint8_t **)(void *)argv[1];
    top_screen = fb[0];
    bottom_screen = fb[2];

    ntrboot_flasher();
    handle_exit();

    return 0;
}

void ntrboot_flasher()
{
    menu_draw_info();
    if (!menu_show_intro_warning())
        return;

    menu_wait_cart_insert();

    Cart_Init();
    uint32_t chip_id = Cart_GetID();
    uint8_t *header = (uint8_t*)malloc(0x1000);
    NTR_CmdReadHeader(header);
    
    reselect_cart: while(selected_flashcart == 0)
    {
        int8_t flashcart_index = menu_select_flashcart();
        if (flashcart_index == -1)
            return;

        selected_flashcart = flashcart_list->at(flashcart_index);
        if (selected_flashcart->initialize())
            break;
        selected_flashcart->shutdown();            

        selected_flashcart = 0;
        ShowPrompt(BOTTOM_SCREEN, false, "Flashcart setup failed");

        // todo: reset the cart and init it again
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
                return;
        }
    }
}

void handle_exit()
{
    while(!i2cWriteRegister(I2C_DEV_MCU, 0x20, 1));
    while(true);
}

void ntrboot_dump_flash() {
    ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);
    DrawStringF(BOTTOM_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Dumping flash");

    ELM_Mount();

    uint32_t length = selected_flashcart->getMaxLength();

    uint8_t* mem = (uint8_t*)malloc(length);    
    if (mem == NULL)
        ShowPrompt(BOTTOM_SCREEN, false, "Failed to allocate memory"); 

    ShowProgress(BOTTOM_SCREEN, 0, 0);
    selected_flashcart->readFlash(0, length, mem);

    ShowPrompt(BOTTOM_SCREEN, false, "Read from flash successfully.");
  

    //Create folder if it doesn't exist
    struct stat st;
    if (stat("fat1:/ntrboot", &st) == -1) {
        mkdir("fat1:/ntrboot", 0700);
    }

    FILE *f = fopen("fat1:/ntrboot/backup.bin","wb");
    fwrite (mem, 1, length, f);
    fclose(f);

    free(mem);
    ShowProgress(BOTTOM_SCREEN, 0, 0);

    DrawStringF(BOTTOM_SCREEN, 10, 150, COLOR_GREEN, STD_COLOR_BG, "Dump Complete!");  
    ELM_Unmount();

    DrawStringF(BOTTOM_SCREEN, 10, 160, STD_COLOR_FONT, STD_COLOR_BG, "Press <A> to return to the main menu.");    

    while(!CheckButton(BUTTON_A));
}

void ntrboot_restore_flash() {
    ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);
    DrawStringF(BOTTOM_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Restoring flash");

    ELM_Mount();

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

        uint32_t flash_length = selected_flashcart->getMaxLength();
        uint8_t* flash_mem = (uint8_t*)memalign(4,flash_length);
        uint8_t* flash_memp = flash_mem;

        DrawStringF(BOTTOM_SCREEN, 10, 40, STD_COLOR_FONT, STD_COLOR_BG, "Reading...");
        ShowProgress(BOTTOM_SCREEN, 0, 0);
        selected_flashcart->readFlash(0, length, flash_memp);

        int written_chunk = 0;
        const int chunk_size = 64*1024;
        for(uint32_t j=0; j<length; j+=chunk_size)
        {
            DrawStringF(BOTTOM_SCREEN, 10, 50, STD_COLOR_FONT, STD_COLOR_BG, "Checking %08X", j);        
            ShowProgress(BOTTOM_SCREEN, j, length);
            if(memcmp(flash_memp+j,memp+j,chunk_size) != 0)
            {
                DrawStringF(BOTTOM_SCREEN, 10, 60, STD_COLOR_FONT, STD_COLOR_BG, "Writing chunk %08X", j);
                selected_flashcart->writeFlash(j, chunk_size, memp+j);
                written_chunk++;
                DrawStringF(BOTTOM_SCREEN, 10, 70, STD_COLOR_FONT, STD_COLOR_BG, "Chunks written %d (%08X)", written_chunk, written_chunk);
            }
        }

        free(flash_mem);

        DrawStringF(BOTTOM_SCREEN, 10, 90, COLOR_GREEN, STD_COLOR_BG, "Restoring Complete!");        

        free(mem);
    }
    else
        DrawStringF(BOTTOM_SCREEN, 10, 40, COLOR_RED, STD_COLOR_BG, "Restore file was not found.");

    ELM_Unmount();

    DrawStringF(BOTTOM_SCREEN, 10, 160, STD_COLOR_FONT, STD_COLOR_BG, "Press <A> to return to the main menu.");    
    while(!CheckButton(BUTTON_A));
}

void ntrboot_inject() {
    ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);
    DrawStringF(BOTTOM_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Injecting Ntrboot");

    ELM_Mount();

    FILE *f = fopen("fat1:/ntrboot/boot9strap_ntr.firm","rb");;
    uint8_t *blowfish_key = (uint8_t*)blowfish_retail_bin;

    // todo: ask for dev unit
    
    if (f != NULL)
    {
        fseek(f, 0, SEEK_END);
        size_t firm_size = ftell(f);
        uint8_t *firm = (uint8_t*)malloc(firm_size);
        fseek(f, 0, SEEK_SET);
        fread(firm, 1, firm_size, f);    
        fclose(f);

        ShowProgress(BOTTOM_SCREEN, 0, 0);
        selected_flashcart->injectNtrBoot((uint8_t *)blowfish_retail_bin, firm, firm_size);
        ShowProgress(BOTTOM_SCREEN, 0, 0);

        DrawStringF(BOTTOM_SCREEN, 10, 40, COLOR_GREEN, STD_COLOR_BG, "Injection Complete!");
    } else
        DrawStringF(BOTTOM_SCREEN, 10, 40, COLOR_RED, STD_COLOR_BG, "/ntrboot/boot9strap_ntr.firm not found");        

    ELM_Unmount();
    
    DrawStringF(BOTTOM_SCREEN, 10, 60, STD_COLOR_FONT, STD_COLOR_BG, "Press <A> to return to the main menu.");        
    while(!CheckButton(BUTTON_A));
}