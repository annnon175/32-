#ifndef OLED_H
#define OLED_H
#include <stdint.h>
extern volatile uint32_t g_i2c_errors;
uint8_t OLED_Probe(void);
void OLED_Init(void);
void OLED_Command(const uint8_t *cmd, uint16_t count);
void OLED_Update(const uint8_t *buffer);
void OLED_Scroll(uint8_t enable);
#endif
