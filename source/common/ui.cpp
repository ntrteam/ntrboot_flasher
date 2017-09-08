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

void DrawRectangle(uint8_t* screen, int x, int y, int width, int height, int color)
{
    for (int yy = 0; yy < height; yy++) {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_HEIGHT);
        int yDisplacement = ((SCREEN_HEIGHT - (y + yy) - 1) * BYTES_PER_PIXEL);
        u8* screenPos = screen + xDisplacement + yDisplacement;
        for (int xx = width - 1; xx >= 0; xx--) {
            *(screenPos + 0) = color >> 16;  // B
            *(screenPos + 1) = color >> 8;   // G
            *(screenPos + 2) = color & 0xFF; // R
            screenPos += BYTES_PER_PIXEL * SCREEN_HEIGHT;
        }
    }
}

void DrawCharacter(uint8_t *screen, int character, int x, int y, int color, int bgcolor)
{
    for (int yy = 0; yy < FONT_HEIGHT; yy++)
    {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_HEIGHT);
        int yDisplacement = ((SCREEN_HEIGHT - (y + yy) - 1) * BYTES_PER_PIXEL);
        uint8_t *screenPos = screen + xDisplacement + yDisplacement;

        uint8_t charPos = font[character * FONT_HEIGHT + yy];
        for (int xx = 7; xx >= (8 - FONT_WIDTH); xx--)
        {
            if ((charPos >> xx) & 1)
            {
                *(screenPos + 0) = color >> 16; //B
                *(screenPos + 1) = color >> 8; //G
                *(screenPos + 2) = color & 0xFF; //R
            } else if (bgcolor != COLOR_TRANSPARENT) {
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
    DrawHex(screen, hex,x + strlen(str) * FONT_WIDTH, y, color, bgcolor);
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

    uint32_t keys = WaitButton(BUTTON_A | BUTTON_B);
    if (keys & BUTTON_A) ret = true;
    if (keys & BUTTON_B) ret = false;

    ClearScreen(screen, STD_COLOR_BG);

    return ret;
}

void ShowProgress(uint8_t *screen, uint32_t current, uint32_t total, const char* status)
{
    const uint8_t bar_width = 240;
    const uint8_t bar_height = 12;
    const uint16_t bar_pos_x = (SCREEN_WIDTH - bar_width) / 2;
    const uint16_t bar_pos_y = (SCREEN_HEIGHT / 2) - (bar_height / 2);
    const uint16_t text_pos_x = bar_pos_x + (bar_width/2) - (FONT_WIDTH*2);
    const uint16_t text_pos_y = bar_pos_y + 1;

    static uint32_t last_prog_width = 0;
    uint32_t prog_width = ((total > 0) && (current <= total)) ? (current * (bar_width-4)) / total : 0;
    uint32_t prog_percent = ((total > 0) && (current <= total)) ? (current * 100) / total : 0;

    DrawString(screen, status, bar_pos_x, bar_pos_y - FONT_HEIGHT - 4, STD_COLOR_FONT, STD_COLOR_BG);

    // draw the initial outline
    if (current == 0 || last_prog_width > prog_width)
    {
        ClearScreen(screen, STD_COLOR_BG);
        DrawRectangle(screen, bar_pos_x, bar_pos_y, bar_width, bar_height, STD_COLOR_FONT);
        DrawRectangle(screen, bar_pos_x + 1, bar_pos_y + 1, bar_width - 2, bar_height - 2, STD_COLOR_BG);
    }

    // only draw the rectangle if it's changed.
    if (current == 0 || last_prog_width != prog_width)
    {
        DrawRectangle(screen, bar_pos_x + 1, bar_pos_y + 1, bar_width - 2, bar_height - 2, STD_COLOR_BG); // Clear the progress bar before re-rendering.
        DrawRectangle(screen, bar_pos_x + 2, bar_pos_y + 2, prog_width, bar_height - 4, COLOR_GREEN);
        DrawStringF(screen, text_pos_x, text_pos_y, STD_COLOR_FONT, COLOR_TRANSPARENT, "%3lu%%", prog_percent);
    }

    last_prog_width = prog_width;
}
