#include "menu.h"

#include <ncgcpp/ntrcard.h>

#include "device.h"

#include "hid.h"
#include "ui.h"
#include "i2c.h"
#include "elm.h"

using flashcart_core::Flashcart;
using flashcart_core::flashcart_list;

void menu_wait_cart_insert()
{
    if (!ncgc::NTRCard::cardInserted())
        ShowString(TOP_SCREEN, "No cartridge inserted\n(insert a cartridge to continue)");
    ncgc::NTRCard::waitForCard();
}

bool menu_show_intro_warning()
{
    ClearScreen(TOP_SCREEN, COLOR_RED);

    DrawString(TOP_SCREEN, 10, 10, COLOR_WHITE, COLOR_RED,
        "WARNING: READ THIS BEFORE CONTINUING\n"
        "------------------------------------\n"
        "\n"
        "If you don't know what you're doing: STOP. Open your browser to\n"
        "http://3ds.guide and follow the steps provided there.\n"
        "\n"
        "This software writes directly to your flashcart. It's possible\n"
        "you may brick your flashcart, rendering it unusable.\n"
        "\n"
        "ALWAYS KEEP A BACKUP\n"
        "\n"
        "<A> Continue  <B> Poweroff  <START> Reboot\n");

    while (HID_STATE != 0); // bug fix: wait for the HID_STATE to reset (luma seems to pass through with A button already down)
    uint32_t state = WaitButton(BUTTON_A | BUTTON_B | BUTTON_START);
    ClearScreen(TOP_SCREEN, COLOR_BLACK);

    if (state & BUTTON_START) i2cReboot();
    return (state & BUTTON_A) == BUTTON_A;
}

void menu_unmount() {
    ELM_Unmount();
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);
    DrawString(TOP_SCREEN, 10, 10, STD_COLOR_FONT, STD_COLOR_BG,
        "It is now safe to remove your SD card.\n"
        "\n"
        "Press <A> to continue after inserting your SD card again.");

    WaitButton(BUTTON_A);
    ELM_Mount();
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);
}

int8_t menu_select_flashcart()
{
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);

    size_t deviceOption = 0;
    while(true)
    {
        DrawRectangle(TOP_SCREEN, 0, 0, SCREEN_WIDTH_TOP, 12, COLOR_BLUE);

        DrawString(TOP_SCREEN, 10,  1, COLOR_WHITE, COLOR_BLUE, "Select your flashcart:");
        DrawString(TOP_SCREEN, SCREEN_WIDTH_TOP - 250, 1, COLOR_WHITE, COLOR_BLUE, "<A> Select  <B> Poweroff  <START> Reboot  <SELECT> Unmount SD");

        DrawRectangle(TOP_SCREEN, 0, SCREEN_HEIGHT-24, SCREEN_WIDTH_TOP, 24, COLOR_LIGHTGREY);

        DrawStringF(TOP_SCREEN, 10, SCREEN_HEIGHT-23, COLOR_BLACK, COLOR_LIGHTGREY, "ntrboot_flasher: %s", NTRBOOT_FLASHER_VERSION);
        DrawStringF(TOP_SCREEN, 10, SCREEN_HEIGHT-11, COLOR_BLACK, COLOR_LIGHTGREY, "flashcart_core:  %s", FLASHCART_CORE_VERSION);

        DrawString(TOP_SCREEN, SCREEN_WIDTH_TOP - 130, SCREEN_HEIGHT-23, COLOR_BLACK, COLOR_LIGHTGREY, "<Y> Toggle log level");
        DrawStringF(TOP_SCREEN, SCREEN_WIDTH_TOP - 130, SCREEN_HEIGHT-11, COLOR_BLACK, COLOR_LIGHTGREY, "Log level: %s", loglevel_str());

        // todo: scroll through this, we can only have 23 on the screen at once
        size_t i = 0;
        for (std::vector<Flashcart*>::iterator it = flashcart_list->begin() ; it != flashcart_list->end(); ++it, i++) {
            DrawStringF(TOP_SCREEN, 10, 20 + i*10, (i == deviceOption) ? COLOR_RED : STD_COLOR_FONT, STD_COLOR_BG, (*it)->getName());

            if (i == deviceOption)
            {
                ClearScreen(BOTTOM_SCREEN, STD_COLOR_BG);

                DrawRectangle(BOTTOM_SCREEN, 0, 0, SCREEN_WIDTH_TOP, 12, COLOR_BLUE);
                DrawString(BOTTOM_SCREEN, 106, 0, COLOR_WHITE, COLOR_BLUE, "Selected Cart Info");

                DrawStringF(BOTTOM_SCREEN, 10, 20, STD_COLOR_FONT, STD_COLOR_BG, "Name: %s", (*it)->getName());
                DrawStringF(BOTTOM_SCREEN, 10, 30, STD_COLOR_FONT, STD_COLOR_BG, "Author: %s", (*it)->getAuthor());

                DrawString(BOTTOM_SCREEN, 10, 50, STD_COLOR_FONT, STD_COLOR_BG, (*it)->getDescription());
            }
        }

        uint32_t keys = WaitButton(BUTTON_ANY);
        if (deviceOption >= 1 && (keys & BUTTON_UP)) deviceOption--;
        if (keys & BUTTON_DOWN) deviceOption++;

        if(deviceOption >= flashcart_list->size()) deviceOption = flashcart_list->size() - 1;

        if (keys & BUTTON_A) return deviceOption;
        if (keys & BUTTON_B) return -1;
        if (keys & BUTTON_Y) toggleLoglevel();
        if (keys & BUTTON_START) {
            ELM_Unmount();
            i2cReboot();
        }
        if (keys & BUTTON_SELECT) menu_unmount();
    }

    return -1;
}

#define MENU_HIGHLIGHT(yn) (yn ? COLOR_RED : STD_COLOR_FONT)

uint8_t menu_flashcart_menu(const char* flashcart_name)
{
    ClearScreen(TOP_SCREEN, STD_COLOR_BG);

    DrawStringF(TOP_SCREEN, 10, 10, STD_COLOR_FONT, STD_COLOR_BG, "Flashcart: %s", flashcart_name);

    int menuOption = 0;
    while(true)
    {
        DrawString(TOP_SCREEN, 10, 30, MENU_HIGHLIGHT(menuOption == MENU_DUMP)             , STD_COLOR_BG, "Dump Flash");
        DrawString(TOP_SCREEN, 10, 40, MENU_HIGHLIGHT(menuOption == MENU_RESTORE)          , STD_COLOR_BG, "Restore Flash");
        DrawString(TOP_SCREEN, 10, 50, MENU_HIGHLIGHT(menuOption == MENU_INJECT)           , STD_COLOR_BG, "Inject Ntrboot");
        DrawString(TOP_SCREEN, 10, 70, MENU_HIGHLIGHT(menuOption == MENU_SELECT_FLASHCART) , STD_COLOR_BG, "Reselect Flashcart");
        DrawString(TOP_SCREEN, 10, 90, MENU_HIGHLIGHT(menuOption == MENU_EXIT)             , STD_COLOR_BG, "Poweroff");
        DrawString(TOP_SCREEN, 10, 100, MENU_HIGHLIGHT(menuOption == MENU_REBOOT)          , STD_COLOR_BG, "Reboot");

        uint32_t keys = WaitButton(BUTTON_ANY);
        if (keys & BUTTON_UP) menuOption--;
        if (keys & BUTTON_DOWN) menuOption++;

        if(menuOption < 0) menuOption = 0;
        else if(menuOption >= MENU_MAX) menuOption = MENU_MAX - 1;

        if (keys & BUTTON_A) return menuOption;
        if (keys & BUTTON_B) return MENU_EXIT;
        if (keys & BUTTON_START) return MENU_REBOOT;
    }

    return -1;
}
