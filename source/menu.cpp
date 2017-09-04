#include "menu.h"

#include "device.h"

#include "hid.h"
#include "protocol.h"
#include "ui.h"

void menu_wait_cart_insert()
{
    if (REG_CARDCONF2 & 0x1)
        ShowString(TOP_SCREEN, "No cartridge inserted\n(insert a cartridge to continue)");
    while (REG_CARDCONF2 & 0x1) { };
}

bool menu_show_intro_warning()
{
    ClearScreen(TOP_SCREEN, COLOR_RED);
    
    DrawStringF(TOP_SCREEN, 10, 10, COLOR_BLACK, COLOR_RED, "ntrboot flasher [%s]", NTRBOOT_FLASHER_VERSION);
    DrawStringF(TOP_SCREEN, 10, 20, COLOR_BLACK, COLOR_RED, "flashcart_core [%s]", FLASHCART_CORE_VERSION);

    DrawStringF(TOP_SCREEN, 10, 40, COLOR_BLACK, COLOR_RED, "WARNING");
    DrawStringF(TOP_SCREEN, 10, 50, COLOR_BLACK, COLOR_RED, "-------");
    DrawStringF(TOP_SCREEN, 10, 70, COLOR_BLACK, COLOR_RED, "This software writes directly to\nyour flashcart to inject ntrboot.");
    DrawStringF(TOP_SCREEN, 10, 90, COLOR_BLACK, COLOR_RED, "In rare cases this may brick your\nflashcart and leave it unusable.");
    DrawStringF(TOP_SCREEN, 10, 130, COLOR_BLACK, COLOR_RED, "<A> Continue <B> Exit");

    return (WaitButton(BUTTON_A | BUTTON_B) & BUTTON_A) == BUTTON_A;
}

int8_t menu_select_flashcart()
{
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);

    int deviceOption = 0;    
    while(true)
    {       
        DrawStringF(TOP_SCREEN, 0, 0, COLOR_WHITE, COLOR_BLUE, "              Select your flashcart:              ");
        DrawStringF(TOP_SCREEN, 0, SCREEN_HEIGHT-FONT_HEIGHT, COLOR_WHITE, COLOR_RED,  "                Press <B> to Exit.                ");        

        // todo: scroll through this, we can only have 23 on the screen at once
        int i = 0;
        for (std::vector<Flashcart*>::iterator it = flashcart_list->begin() ; it != flashcart_list->end(); ++it, i++) {
            DrawStringF(TOP_SCREEN, 10, 20 + i*10, (i == deviceOption) ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, (*it)->getName());

            if (i == deviceOption)
            {
                ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);

                DrawStringF(BOTTOM_SCREEN, 0, 0, COLOR_WHITE, COLOR_BLUE, "           Selected Cart Info           ");
                DrawStringF(BOTTOM_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Name: %s", (*it)->getName());
                DrawStringF(BOTTOM_SCREEN, 10, 30, STD_COLOR_FONT, STD_COLOR_BG, "Author: %s", (*it)->getAuthor());

                DrawStringF(BOTTOM_SCREEN, 10, 50, STD_COLOR_FONT, STD_COLOR_BG, "Description:");
                DrawStringF(BOTTOM_SCREEN, 10, 70, STD_COLOR_FONT, STD_COLOR_BG, (*it)->getDescription());                
                
            }
        }

        uint32_t keys = WaitButton(BUTTON_ANY);
        if (keys & BUTTON_UP) deviceOption--;
        if (keys & BUTTON_DOWN) deviceOption++;

        if(deviceOption < 0) deviceOption = 0;
        else if(deviceOption >= flashcart_list->size()) deviceOption = flashcart_list->size() - 1;

        if (keys & BUTTON_A) return deviceOption;
        if (keys & BUTTON_B) return -1;        
    }

    return -1;
}


uint8_t menu_flashcart_menu(const char* flashcart_name)
{
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);    

    DrawStringF(TOP_SCREEN, 10, 10, STD_COLOR_FONT, STD_COLOR_BG, "Flashcart: %s", flashcart_name);

    int menuOption = 0;    
    while(true)
    {        
        DrawStringF(TOP_SCREEN, 10, 30, (menuOption == MENU_DUMP   )          ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Dump Flash");
        DrawStringF(TOP_SCREEN, 10, 40, (menuOption == MENU_RESTORE)          ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Restore Flash");
        DrawStringF(TOP_SCREEN, 10, 50, (menuOption == MENU_INJECT )          ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Inject Ntrboot");
        DrawStringF(TOP_SCREEN, 10, 70, (menuOption == MENU_SELECT_FLASHCART) ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Reselect Flashcart");
        DrawStringF(TOP_SCREEN, 10, 90, (menuOption == MENU_EXIT)             ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Exit");
        
        uint32_t keys = WaitButton(BUTTON_ANY);
        if (keys & BUTTON_UP) menuOption--;
        if (keys & BUTTON_DOWN) menuOption++;

        if(menuOption < 0) menuOption = 0;
        else if(menuOption >= MENU_MAX) menuOption = MENU_MAX - 1;

        if (keys & BUTTON_A) return menuOption;
        if (keys & BUTTON_B) return MENU_EXIT;
    }

    return -1;
}