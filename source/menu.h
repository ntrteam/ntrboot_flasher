#pragma once

#include "common.h"

enum
{
    MENU_DUMP=0,
    MENU_RESTORE,
    MENU_INJECT,
    MENU_SELECT_FLASHCART,
    MENU_EXIT,
    MENU_REBOOT,
    MENU_MAX
};

bool menu_show_intro_warning();
void menu_wait_cart_insert();
int8_t menu_select_flashcart();
uint8_t menu_flashcart_menu(const char* flashcart_name);

// From platform.cpp
char const *loglevel_str(void);
void toggleLoglevel(void);
