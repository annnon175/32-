#include "gfx.h"
#include "u8g2.h"
#ifndef DESKTOP_PREVIEW
#include "u8g2_port.h"
#endif

static u8g2_t display;
extern const uint8_t font_lab12[];
#ifdef DESKTOP_PREVIEW
static uint8_t desktop_byte(u8x8_t *u, uint8_t msg, uint8_t arg, void *p)
{ (void)u; (void)msg; (void)arg; (void)p; return 1; }
#define u8x8_hal_i2c desktop_byte
#define u8x8_hal_delay desktop_byte
#endif

void Gfx_Init(void)
{
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&display, U8G2_R0,
                                          u8x8_hal_i2c, u8x8_hal_delay);
#ifndef DESKTOP_PREVIEW
    U8g2_PortAddress(&display);
#endif
    u8g2_InitDisplay(&display);
    u8g2_SetPowerSave(&display, 0);
    u8g2_SetFont(&display, font_lab12);
    u8g2_SetFontMode(&display, 1);
}
void Gfx_Clear(void) { u8g2_ClearBuffer(&display); }
void Gfx_Send(void)
{
#ifndef DESKTOP_PREVIEW
    u8g2_SendBuffer(&display);
#endif
}
uint8_t *Gfx_Buffer(void) { return u8g2_GetBufferPtr(&display); }
void Gfx_Text(int16_t x, int16_t y, const char *text)
{ u8g2_DrawUTF8(&display, x, y + 11, text); }
int16_t Gfx_TextWidth(const char *text)
{ return (int16_t)u8g2_GetUTF8Width(&display, text); }
void Gfx_Pixel(int16_t x, int16_t y) { u8g2_DrawPixel(&display, x, y); }
void Gfx_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{ u8g2_DrawLine(&display, x0, y0, x1, y1); }
void Gfx_Box(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t fill)
{
    if (w <= 0 || h <= 0) return;
    if (fill) u8g2_DrawBox(&display, x, y, w, h);
    else u8g2_DrawFrame(&display, x, y, w, h);
}
void Gfx_Circle(int16_t cx, int16_t cy, int16_t radius)
{ u8g2_DrawCircle(&display, cx, cy, radius, U8G2_DRAW_ALL); }
void Gfx_Bitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t *bits)
{ u8g2_DrawXBM(&display, x, y, w, h, bits); }
