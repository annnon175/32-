#include "main.h"
#include "u8g2_port.h"
#include "lab_config.h"
#include "oled.h"
#include <string.h>

void U8g2_PortAddress(u8g2_t *display)
{
    /* U8g2和STM32 HAL都使用左移后的地址：0x3C -> 0x78。 */
    u8g2_SetI2CAddress(display, OLED_ADDR << 1);
}

uint8_t u8x8_hal_i2c(u8x8_t *u, uint8_t msg, uint8_t arg, void *ptr)
{
    static uint8_t packet[256];
    static uint16_t used;
    switch (msg) {
    case U8X8_MSG_BYTE_INIT: return 1;
    case U8X8_MSG_BYTE_START_TRANSFER: used = 0; return 1;
    case U8X8_MSG_BYTE_SEND:
        if (used + arg > sizeof(packet)) return 0;
        memcpy(packet + used, ptr, arg);
        used += arg;
        return 1;
    case U8X8_MSG_BYTE_END_TRANSFER:
        if (HAL_I2C_Master_Transmit(&hi2c1, u8x8_GetI2CAddress(u), packet,
                                  used, I2C_TIMEOUT_MS) != HAL_OK) {
            ++g_i2c_errors;
            return 0;
        }
        return 1;
    case U8X8_MSG_BYTE_SET_DC: return 1;
    default: return 0;
    }
}

uint8_t u8x8_hal_delay(u8x8_t *u, uint8_t msg, uint8_t arg, void *ptr)
{
    (void)u; (void)ptr;
    if (msg == U8X8_MSG_DELAY_MILLI) HAL_Delay(arg);
    /* 硬件I2C产生SCL，不用GPIO回调手动翻转SCL/SDA，也不需要TIM心跳。 */
    return 1;
}
