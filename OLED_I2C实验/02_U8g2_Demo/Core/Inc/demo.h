#ifndef DEMO_H
#define DEMO_H
#include <stdint.h>
typedef struct {
    int32_t temperature_x10;
    uint16_t humidity_x10;
    uint8_t sensor_valid;
    uint32_t sample_age_ms;
} DemoData;
void Demo_Draw(uint8_t page, uint32_t elapsed_ms, const DemoData *data);
#endif
