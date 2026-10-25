#ifndef U8G2_PORT_H
#define U8G2_PORT_H
#include "u8g2.h"
void U8g2_PortAddress(u8g2_t *display);
uint8_t u8x8_hal_i2c(u8x8_t *u, uint8_t msg, uint8_t arg, void *ptr);
uint8_t u8x8_hal_delay(u8x8_t *u, uint8_t msg, uint8_t arg, void *ptr);
#endif
