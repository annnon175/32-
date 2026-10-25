#include "gfx.h"
#include "font_raw.h"
#include <string.h>
#ifndef DESKTOP_PREVIEW
#include "oled.h"
#endif

static uint8_t framebuffer[1024];
uint8_t *Gfx_Buffer(void) { return framebuffer; }
void Gfx_Init(void)
{
#ifndef DESKTOP_PREVIEW
    OLED_Init();
#endif
}
void Gfx_Clear(void) { memset(framebuffer, 0, sizeof(framebuffer)); }
void Gfx_Send(void)
{
#ifndef DESKTOP_PREVIEW
    OLED_Update(framebuffer);
#endif
}

void Gfx_Pixel(int16_t x, int16_t y)
{
    /* SSD1306一页8行，1个字节竖着放8个像素，最低位在上。 */
    if (x >= 0 && x < 128 && y >= 0 && y < 64)
        framebuffer[(y / 8) * 128 + x] |= (uint8_t)(1U << (y % 8));
}

void Gfx_Bitmap(int16_t x, int16_t y, uint8_t w, uint8_t h, const uint8_t *bits)
{
    uint8_t row, col;
    uint8_t stride = (w + 7U) / 8U;
    for (row = 0; row < h; ++row)
        for (col = 0; col < w; ++col)
            if (bits[row * stride + col / 8] & (1U << (col % 8)))
                Gfx_Pixel(x + col, y + row);
}

void Gfx_Line(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    int16_t dx = x1 > x0 ? x1 - x0 : x0 - x1;
    int16_t dy = y1 > y0 ? y0 - y1 : y1 - y0;
    int16_t sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
    int16_t err = dx + dy, twice;
    for (;;) {
        Gfx_Pixel(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        twice = 2 * err;
        if (twice >= dy) { err += dy; x0 += sx; }
        if (twice <= dx) { err += dx; y0 += sy; }
    }
}

void Gfx_Box(int16_t x, int16_t y, int16_t w, int16_t h, uint8_t fill)
{
    int16_t row;
    if (w <= 0 || h <= 0) return;
    if (fill) {
        for (row = 0; row < h; ++row) Gfx_Line(x, y + row, x + w - 1, y + row);
    } else {
        Gfx_Line(x, y, x + w - 1, y);
        Gfx_Line(x, y + h - 1, x + w - 1, y + h - 1);
        Gfx_Line(x, y, x, y + h - 1);
        Gfx_Line(x + w - 1, y, x + w - 1, y + h - 1);
    }
}

void Gfx_Circle(int16_t cx, int16_t cy, int16_t radius)
{
    int16_t x = radius, y = 0, error = 1 - radius;
    while (x >= y) {
        Gfx_Pixel(cx + x, cy + y); Gfx_Pixel(cx - x, cy + y);
        Gfx_Pixel(cx + x, cy - y); Gfx_Pixel(cx - x, cy - y);
        Gfx_Pixel(cx + y, cy + x); Gfx_Pixel(cx - y, cy + x);
        Gfx_Pixel(cx + y, cy - x); Gfx_Pixel(cx - y, cy - x);
        ++y;
        if (error < 0) error += 2 * y + 1;
        else { --x; error += 2 * (y - x) + 1; }
    }
}

static uint16_t next_unicode(const uint8_t **ptr)
{
    const uint8_t *s = *ptr;
    uint16_t code;
    if (*s < 0x80) { code = *s; *ptr = s + 1; }
    else if ((*s & 0xE0) == 0xC0) {
        code = (uint16_t)((s[0] & 0x1F) << 6) | (s[1] & 0x3F);
        *ptr = s + 2;
    } else {
        code = (uint16_t)((s[0] & 0x0F) << 12) |
               (uint16_t)((s[1] & 0x3F) << 6) | (s[2] & 0x3F);
        *ptr = s + 3;
    }
    return code;
}

static const FontGlyph *find_glyph(uint16_t code)
{
    uint16_t i;
    for (i = 0; i < FONT_GLYPH_COUNT; ++i)
        if (font_glyphs[i].unicode == code) return &font_glyphs[i];
    return &font_glyphs['?' - 32];
}

void Gfx_Text(int16_t x, int16_t y, const char *text)
{
    const uint8_t *s = (const uint8_t *)text;
    const FontGlyph *g;
    uint8_t row, col;
    while (*s) {
        g = find_glyph(next_unicode(&s));
        for (row = 0; row < g->height; ++row)
            for (col = 0; col < g->width; ++col)
                if (g->rows[row] & (0x8000U >> col))
                    Gfx_Pixel(x + g->x_offset + col,
                              y + 11 - g->y_offset - g->height + row);
        x += g->advance;
    }
}

int16_t Gfx_TextWidth(const char *text)
{
    const uint8_t *s = (const uint8_t *)text;
    int16_t width = 0;
    while (*s) width += find_glyph(next_unicode(&s))->advance;
    return width;
}
