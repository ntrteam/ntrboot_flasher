#pragma once

extern uint8_t *top_screen, *bottom_screen;

#define TOP_SCREEN ((uint8_t*)top_screen)
#define BOTTOM_SCREEN ((uint8_t*)bottom_screen)

#define BYTES_PER_PIXEL 3
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH_TOP 400
#define SCREEN_WIDTH_BOT 320

#define SCREEN_SIZE (BYTES_PER_PIXEL*SCREEN_WIDTH*SCREEN_HEIGHT)

#define RGB(r,g,b) (r<<24|b<<16|g<<8|r)

#define COLOR_BLACK         RGB(0x00, 0x00, 0x00)
#define COLOR_WHITE         RGB(0xFF, 0xFF, 0xFF)
#define COLOR_RED           RGB(0xFF, 0x00, 0x00)
#define COLOR_GREEN         RGB(0x00, 0xFF, 0x00)
#define COLOR_BLUE          RGB(0x00, 0x00, 0xFF)
#define COLOR_CYAN          RGB(0x00, 0xFF, 0xFF)
#define COLOR_MAGENTA       RGB(0xFF, 0x00, 0xFF)
#define COLOR_YELLOW        RGB(0xFF, 0xFF, 0x00)
#define COLOR_GREY          RGB(0x77, 0x77, 0x77)
#define COLOR_TRANSPARENT   RGB(0xFF, 0x00, 0xEF) // otherwise known as 'super fuchsia'

#define COLOR_GREYBLUE      RGB(0xA0, 0xA0, 0xFF)
#define COLOR_GREYGREEN     RGB(0xA0, 0xFF, 0xA0)
#define COLOR_GREYRED       RGB(0xFF, 0xA0, 0xA0)
#define COLOR_GREYCYAN      RGB(0xA0, 0xFF, 0xFF)
#define COLOR_TINTEDRED     RGB(0xFF, 0x60, 0x60)
#define COLOR_LIGHTGREY     RGB(0xA0, 0xA0, 0xA0)

#define COLOR_ASK           COLOR_GREYGREEN
#define COLOR_SELECT        COLOR_LIGHTGREY
#define COLOR_ACCENT        COLOR_GREEN

#define STD_COLOR_BG   COLOR_BLACK
#define STD_COLOR_FONT COLOR_WHITE

#define DBG_COLOR_BG   COLOR_BLACK
#define DBG_COLOR_FONT COLOR_WHITE

#define DBG_START_Y 10
#define DBG_END_Y   (SCREEN_HEIGHT - 10)
#define DBG_START_X 10
#define DBG_END_X   (SCREEN_WIDTH_TOP - 10)
#define DBG_STEP_Y  10

void ClearScreen(unsigned char *screen, int color);
void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor);
void DrawHex(unsigned char *screen, unsigned int hex, int x, int y, int color, int bgcolor);
void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor);
void DrawHexWithName(unsigned char *screen, const char *str, unsigned int hex, int x, int y, int color, int bgcolor);
void DrawStringF(unsigned char *screen, int x, int y, const char *format, ...);
void ShowProgress(unsigned char *screen, uint32_t current, uint32_t total);

