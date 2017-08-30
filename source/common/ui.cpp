#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "font.h"
#include "hid.h"
#include "ui.h"

void ClearScreen(uint8_t *screen, int color)
{
    int width = (SCREEN_WIDTH * SCREEN_HEIGHT);
    for (int i = 0; i < width; i++)
    {
        *(screen++) = color >> 16; //B
        *(screen++) = color >> 8; //G
        *(screen++) = color & 0xFF; //R
    }
}

void DrawCharacter(uint8_t *screen, int character, int x, int y, int color, int bgcolor)
{
    for (int yy = 0; yy < 8; yy++)
    {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_HEIGHT);
        int yDisplacement = ((SCREEN_HEIGHT - (y + yy) - 1) * BYTES_PER_PIXEL);
        uint8_t *screenPos = screen + xDisplacement + yDisplacement;

        uint8_t charPos = font[character * 8 + yy];
        for (int xx = 7; xx >= 0; xx--)
        {
            if ((charPos >> xx) & 1)
            {
                *(screenPos + 0) = color >> 16; //B
                *(screenPos + 1) = color >> 8; //G
                *(screenPos + 2) = color & 0xFF; //R
            }
            else
            {
                *(screenPos + 0) = bgcolor >> 16; //B
                *(screenPos + 1) = bgcolor >> 8; //G
                *(screenPos + 2) = bgcolor & 0xFF; //R
            }
            screenPos += BYTES_PER_PIXEL * SCREEN_HEIGHT;
        }
    }
}

void DrawString(uint8_t* screen, const char *str, int x, int y, int color, int bgcolor)
{
    size_t max_len = (SCREEN_WIDTH - x) / FONT_WIDTH;
    size_t len = (strlen(str) > max_len) ? max_len : strlen(str);
    for (size_t i = 0; i < len; i++)
        DrawCharacter(screen, str[i], x + i * FONT_WIDTH, y, color, bgcolor);
}

void DrawStringF(uint8_t *screen, int x, int y, int color, int bgcolor, const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsprintf(str, format, va);
    va_end(va);

    for (char* text = strtok(str, "\n"); text != NULL; text = strtok(NULL, "\n"), y += 10)
        DrawString(screen, text, x, y, color, bgcolor);
}

void DrawHex(uint8_t *screen, unsigned int hex, int x, int y, int color, int bgcolor)
{
    for(int i=0; i<8; i++)
    {
        int character = '-';
        int nibble = (hex >> ((7-i)*4))&0xF;
        if(nibble > 9) character = 'A' + nibble-10;
        else character = '0' + nibble;
        
        DrawCharacter(screen, character, x+(i*8), y, color, bgcolor);
    }
}

void DrawHexWithName(uint8_t *screen, const char *str, unsigned int hex, int x, int y, int color, int bgcolor)
{
    DrawString(screen, str, x, y, color, bgcolor);
    DrawHex(screen, hex,x + strlen(str) * 8, y, color, bgcolor);
}

void ShowProgress(uint8_t *screen, uint32_t current, uint32_t total)
{
    const uint32_t progX = SCREEN_WIDTH - 40;
    const uint32_t progY = SCREEN_HEIGHT - 20;
    
    if (total > 0) {
        char progStr[8];
        snprintf(progStr, 8, "%3lu%%", (current * 100) / total);
        DrawString(screen, progStr, progX, progY, STD_COLOR_FONT, STD_COLOR_BG);
    } else {
        DrawString(screen, "    ", progX, progY, STD_COLOR_FONT, STD_COLOR_BG);
    }
}

uint32_t GetDrawStringHeight(const char* str) {
    uint32_t height = FONT_HEIGHT;
    for (char* lf = strchr(str, '\n'); (lf != NULL); lf = strchr(lf + 1, '\n'))
        height += 10;
    return height;
}

uint32_t GetDrawStringWidth(const char* str) {
    uint32_t width = 0;
    char* old_lf = (char*) str;
    char* str_end = (char*) str + strnlen(str, 256);
    for (char* lf = strchr(str, '\n'); lf != NULL; lf = strchr(lf + 1, '\n')) {
        if ((uint32_t) (lf - old_lf) > width) width = lf - old_lf;
        old_lf = lf;
    }
    if ((uint32_t) (str_end - old_lf) > width)
        width = str_end - old_lf;
    width *= FONT_WIDTH;
    return width;
}

void ShowString(uint8_t *screen, const char *format, ...)
{
    uint32_t str_width, str_height;
    uint32_t x, y;
    
    char str[256] = { 0 };
    va_list va;
    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);
    
    str_width = GetDrawStringWidth(str);
    str_height = GetDrawStringHeight(str);
    x = (str_width >= SCREEN_WIDTH) ? 0 : (SCREEN_WIDTH - str_width) / 2;
    y = (str_height >= SCREEN_HEIGHT) ? 0 : (SCREEN_HEIGHT - str_height) / 2;
    
    ClearScreen(screen, STD_COLOR_BG);
    DrawStringF(screen, x, y, STD_COLOR_FONT, STD_COLOR_BG, str);
}

bool ShowPrompt(uint8_t *screen, bool ask, const char *format, ...)
{
    uint32_t str_width, str_height;
    uint32_t x, y;
    bool ret = true;
    
    char str[256] = { 0 };
    va_list va;
    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);
    
    str_width = GetDrawStringWidth(str);
    str_height = GetDrawStringHeight(str) + (2 * 10);
    if (str_width < 18 * FONT_WIDTH) str_width = 18 * FONT_WIDTH;
    x = (str_width >= SCREEN_WIDTH) ? 0 : (SCREEN_WIDTH - str_width) / 2;
    y = (str_height >= SCREEN_HEIGHT) ? 0 : (SCREEN_HEIGHT - str_height) / 2;
    
    ClearScreen(screen, STD_COLOR_BG);
    DrawStringF(screen, x, y, STD_COLOR_FONT, STD_COLOR_BG, str);
    DrawStringF(screen, x, y + str_height - (1*10), STD_COLOR_FONT, STD_COLOR_BG, (ask) ? "(<A> yes, <B> no)" : "(<A> to continue)");
    
    while (true) {
        if (CheckButton(BUTTON_A)) break;
        if (CheckButton(BUTTON_B)) {
            ret = false;
            break;
        }
    }
    
    ClearScreen(screen, STD_COLOR_BG);
    
    return ret;
}