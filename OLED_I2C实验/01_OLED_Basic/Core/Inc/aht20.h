#ifndef AHT20_H
#define AHT20_H
#include <stdint.h>
typedef struct {
    int32_t temperature_x10;
    uint16_t humidity_x10;
    uint32_t last_ok;
    uint8_t valid;
} AHT20_Data;
extern AHT20_Data g_aht20;
extern volatile uint32_t g_sensor_errors;
void AHT20_Init(void);
void AHT20_Task(uint32_t now);
uint8_t AHT20_Decode(const uint8_t raw[7], int32_t *t, uint16_t *rh);
#endif
