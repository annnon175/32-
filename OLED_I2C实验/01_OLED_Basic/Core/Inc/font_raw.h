#ifndef FONT_RAW_H
#define FONT_RAW_H
#include <stdint.h>
typedef struct {
    uint16_t unicode;
    uint8_t advance, width, height;
    int8_t x_offset, y_offset;
    uint16_t rows[14];
} FontGlyph;
#define FONT_GLYPH_COUNT 158
extern const FontGlyph font_glyphs[FONT_GLYPH_COUNT];
#endif
