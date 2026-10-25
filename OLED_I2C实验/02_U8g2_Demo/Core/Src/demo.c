#include "demo.h"
#include "gfx.h"
#include "lab_config.h"
#include "avatar_bitmap.h"
#include <string.h>

/* OLED只显示小整数，自己转数字就够了，不引入printf和堆内存。 */
static char *number(char *out, uint32_t value)
{
    char reversed[10];
    uint8_t count = 0;
    do { reversed[count++] = (char)('0' + value % 10U); value /= 10U; } while (value);
    while (count) *out++ = reversed[--count];
    *out = 0;
    return out;
}

static void value_text(char *text, const char *prefix, int32_t value, const char *unit)
{
    char *out;
    strcpy(text, prefix);
    out = text + strlen(text);
    if (value < 0) { *out++ = '-'; value = -value; }
    out = number(out, (uint32_t)value / 10U);
    *out++ = '.';
    *out++ = (char)('0' + value % 10);
    strcpy(out, unit);
}

/* 画图只放在这个文件里，I2C和传感器放在各自驱动里。y是文字顶部。 */
static void title(const char *text)
{
    Gfx_Text(2, 0, text);
    Gfx_Line(0, 14, 127, 14);
}

static void identity(void)
{
    title("OLED / I2C 实验");
    Gfx_Text(2, 18, "姓名：" STUDENT_NAME);
    Gfx_Text(2, 34, "学号：");
    Gfx_Text(2, 49, STUDENT_ID);
}

static void sensor(const DemoData *data)
{
    char text[32];
    int32_t t = data->temperature_x10;
    title("AHT20 温湿度");
    if (!data->sensor_valid) {
        Gfx_Text(2, 22, "AHT20 无数据");
        Gfx_Text(2, 43, "检查接线与供电");
        return;
    }
    /* 用整数的十分之一单位，避免printf浮点库占Flash。 */
    value_text(text, "T: ", t, " C");
    Gfx_Text(2, 21, text);
    value_text(text, "RH: ", data->humidity_x10, " %");
    Gfx_Text(2, 37, text);
    Gfx_Text(2, 52, data->sample_age_ms > 5000U ? "数据超时" : "每2秒更新");
}

static void horizontal(uint32_t ms)
{
    int16_t width = Gfx_TextWidth(SCROLL_TEXT);
    int16_t x = 128 - (int16_t)((ms / 40U) % (uint32_t)(width + 144));
    title("长句左右滚动");
    Gfx_Text(x, 26, SCROLL_TEXT);
    Gfx_Text(2, 50, "UTF-8 中文点阵");
}

static void vertical(uint32_t ms)
{
    int16_t y = 64 - (int16_t)((ms / 60U) % 144U);
    Gfx_Text(2, y, "Hello，欢迎来到");
    Gfx_Text(2, y + 16, "重庆交通大学");
    Gfx_Text(2, y + 32, "物联网205实训室！");
    Gfx_Text(2, y + 48, STUDENT_NAME);
    Gfx_Text(2, y + 64, STUDENT_ID);
}

static void hardware_scroll(void)
{
    /* 显存中的长句分行排好，再由SSD1306的0x26命令连续滚动。 */
    Gfx_Text(2, 0, "SSD1306 硬件滚动");
    Gfx_Text(2, 16, "Hello，欢迎来到");
    Gfx_Text(2, 32, "重庆交通大学");
    Gfx_Text(2, 48, "物联网205实训室！");
}

static void graphics(uint32_t ms)
{
    char text[16];
    uint16_t progress = (uint16_t)((ms / 60U) % 101U);
    title("U8g2 图形演示");
    Gfx_Box(3, 21, 34, 25, 0);
    Gfx_Line(3, 21, 36, 45);
    Gfx_Line(3, 45, 36, 21);
    Gfx_Circle(58, 34, 12);
    Gfx_Box(81, 24, 16, 20, 1);
    strcpy(number(text, progress), "%");
    Gfx_Text(100, 27, text);
    Gfx_Box(0, 52, 128, 10, 0);
    Gfx_Box(2, 54, (int16_t)(progress * 124U / 100U), 6, 1);
}

static void avatar(uint32_t ms)
{
    static const int8_t movement[] = {0, 1, 2, 1, 0, -1, -2, -1};
    uint8_t view = (uint8_t)((ms / 4000U) % 2U);
    int16_t x = 3 + movement[(ms / 250U) % 8U];
    /* XBM按行取模，每字节低位对应左侧像素，别与SSD1306显存页序混用。 */
    Gfx_Bitmap(x, 0, AVATAR_WIDTH, AVATAR_HEIGHT, avatar_bitmap[view]);
    Gfx_Text(72, 3, "卡通头像");
    Gfx_Text(72, 21, STUDENT_NAME);
    Gfx_Text(72, 38, "632407");
    Gfx_Text(72, 51, "030510");
}

void Demo_Draw(uint8_t page, uint32_t ms, const DemoData *data)
{
    Gfx_Clear();
    switch (page) {
    case 1: identity(); break;
    case 2: sensor(data); break;
    case 3: horizontal(ms); break;
    case 4: vertical(ms); break;
    case 5: hardware_scroll(); break;
    case 6: graphics(ms); break;
    case 7: avatar(ms); break;
    default: identity(); break;
    }
}
