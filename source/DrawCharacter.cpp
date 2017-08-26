#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "font.h"
#include "DrawCharacter.h"

void ClearScreen(unsigned char *screen, int color)
{
    unsigned char *screenPos = screen;
    for (int i = 0; i < (SCREEN_HEIGHT * SCREEN_WIDTH_TOP); i++)
    {
        *(screenPos++) = color >> 16; //B
        *(screenPos++) = color >> 8; //G
        *(screenPos++) = color & 0xFF; //R
    }
    
    //memset(screen,color,SCREEN_SIZE);
    //memset(screen + SCREEN_SIZE + 16,color,SCREEN_SIZE);
}

void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor)
{
    for (int yy = 0; yy < 8; yy++)
    {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_HEIGHT);
        int yDisplacement = ((SCREEN_HEIGHT - (y + yy) - 1) * BYTES_PER_PIXEL);
        unsigned char *screenPos = screen + xDisplacement + yDisplacement;

        unsigned char charPos = font[character * 8 + yy];
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

void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor)
{
    for (int i = 0; i < (int)strlen(str); i++)
    {
        DrawCharacter(screen, str[i], x + i * 8, y, color, bgcolor);
    }
}

void DrawHex(unsigned char *screen, unsigned int hex, int x, int y, int color, int bgcolor)
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

void DrawHexWithName(unsigned char *screen, const char *str, unsigned int hex, int x, int y, int color, int bgcolor)
{
    DrawString(screen, str, x, y, color, bgcolor);
    DrawHex(screen, hex,x + strlen(str) * 8, y, color, bgcolor);
}

void DrawStringF(unsigned char *screen, int x, int y, const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsprintf(str, format, va);
    va_end(va);

    DrawString(screen, str, x, y, DBG_COLOR_FONT, DBG_COLOR_BG);
}

void ShowProgress(unsigned char *screen, uint32_t current, uint32_t total)
{
    const uint32_t progX = SCREEN_WIDTH_TOP - 40;
    const uint32_t progY = SCREEN_HEIGHT - 20;
    
    if (total > 0) {
        char progStr[8];
        snprintf(progStr, 8, "%3lu%%", (current * 100) / total);
        DrawString(screen, progStr, progX, progY, DBG_COLOR_FONT, DBG_COLOR_BG);
    } else {
        DrawString(screen, "    ", progX, progY, DBG_COLOR_FONT, DBG_COLOR_BG);
    }
}