#ifndef GFX_H
#define GFX_H
#include <stdint.h>
void Gfx_Init(void);
void Gfx_Clear(void);
void Gfx_Send(void);
void Gfx_Text(int16_t x, int16_t y, const char *text);
int16_t Gfx_TextWidth(const char *text);
void Gfx_Pixel(int16_t x, int16_t y);
void Gfx_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
void Gfx_Box(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t fill);
void Gfx_Circle(int16_t cx, int16_t cy, int16_t radius);
void Gfx_Bitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t *bits);
uint8_t *Gfx_Buffer(void);
#endif
