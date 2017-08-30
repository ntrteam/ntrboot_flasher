#include "menu.h"

#include "device.h"

#include "hid.h"
#include "protocol.h"
#include "ui.h"

void menu_draw_info()
{
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);

    DrawStringF(TOP_SCREEN, 10, 10, STD_COLOR_FONT, STD_COLOR_BG, "ntrboot flasher [%s]", NTRBOOT_FLASHER_VERSION);
    DrawStringF(TOP_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "flashcart_core [%s]", FLASHCART_CORE_VERSION);
}

void menu_wait_cart_insert()
{
    if (REG_CARDCONF2 & 0x1)
        ShowString(BOTTOM_SCREEN, "No cartridge inserted\n(insert a cartridge to continue)");
    while (REG_CARDCONF2 & 0x1) { };
}

bool menu_show_intro_warning()
{
    ClearScreen(BOTTOM_SCREEN, COLOR_RED);
    
    DrawStringF(BOTTOM_SCREEN, 10, 10, COLOR_BLACK, COLOR_RED, "WARNING");
    DrawStringF(BOTTOM_SCREEN, 10, 20, COLOR_BLACK, COLOR_RED, "-------");
    DrawStringF(BOTTOM_SCREEN, 10, 40, COLOR_BLACK, COLOR_RED, "This software writes directly to\nyour flashcart to inject ntrboot.");
    DrawStringF(BOTTOM_SCREEN, 10, 60, COLOR_BLACK, COLOR_RED, "In rare cases this may brick your\nflashcart and leave it unusable.");
    DrawStringF(BOTTOM_SCREEN, 10, 100, COLOR_BLACK, COLOR_RED, "<A> Continue <B> Exit");

    return (WaitButton(BUTTON_A | BUTTON_B) & BUTTON_A) == BUTTON_A;
}

int8_t menu_select_flashcart()
{
    ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);

    int deviceOption = 0;    
    while(true)
    {       
        DrawStringF(BOTTOM_SCREEN, 0, 0, COLOR_WHITE, COLOR_BLUE, "         Select your flashcart:         ");

        // todo: scroll through this, we can only have 23 on the screen at once
        int i = 0;
        for (std::vector<Flashcart*>::iterator it = flashcart_list->begin() ; it != flashcart_list->end(); ++it, i++) {
            DrawStringF(BOTTOM_SCREEN, 10, 20 + i*10, (i == deviceOption) ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, (*it)->getName());
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
    ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);    

    DrawStringF(BOTTOM_SCREEN, 10, 10, STD_COLOR_FONT, STD_COLOR_BG, "Flashcart: %s", flashcart_name);

    int menuOption = 0;    
    while(true)
    {        
        DrawStringF(BOTTOM_SCREEN, 10, 30, (menuOption == MENU_DUMP   )          ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Dump Flash");
        DrawStringF(BOTTOM_SCREEN, 10, 40, (menuOption == MENU_RESTORE)          ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Restore Flash");
        DrawStringF(BOTTOM_SCREEN, 10, 50, (menuOption == MENU_INJECT )          ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Inject Ntrboot");
        DrawStringF(BOTTOM_SCREEN, 10, 70, (menuOption == MENU_SELECT_FLASHCART) ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Reselect Flashcart");
        DrawStringF(BOTTOM_SCREEN, 10, 90, (menuOption == MENU_EXIT)             ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, "Exit");
        
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