#include "main.h"
#include "oled.h"
#include "lab_config.h"
#include <string.h>

volatile uint32_t g_i2c_errors;

uint8_t OLED_Probe(void)
{
    return HAL_I2C_IsDeviceReady(&hi2c1, OLED_ADDR << 1, 1, I2C_TIMEOUT_MS) == HAL_OK;
}

void OLED_Command(const uint8_t *cmd, uint16_t count)
{
    /* 0x00表示后面都是命令，0x40表示后面都是显存数据。 */
    uint8_t packet[33];
    if (count > 32) return;
    packet[0] = 0x00;
    memcpy(packet + 1, cmd, count);
    if (HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR << 1, packet,
                              count + 1, I2C_TIMEOUT_MS) != HAL_OK) ++g_i2c_errors;
}

void OLED_Init(void)
{
    static const uint8_t init[] = {
        0xAE, 0xD5, 0x80, 0xA8, 0x3F, 0xD3, 0x00, 0x40,
        0x8D, 0x14, 0x20, 0x00, 0xA1, 0xC8, 0xDA, 0x12,
        0x81, 0x7F, 0xD9, 0xF1, 0xDB, 0x40, 0xA4, 0xA6, 0x2E, 0xAF
    };
    OLED_Command(init, sizeof(init));
}

void OLED_Update(const uint8_t *buffer)
{
    uint16_t offset;
    uint8_t packet[33];
    static const uint8_t range[] = {0x20, 0x00, 0x21, 0, 127, 0x22, 0, 7};
    OLED_Command(range, sizeof(range));
    packet[0] = 0x40;
    for (offset = 0; offset < 1024; offset += 32) {
        memcpy(packet + 1, buffer + offset, 32);
        if (HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDR << 1, packet,
                                  sizeof(packet), I2C_TIMEOUT_MS) != HAL_OK) {
            ++g_i2c_errors;
            break;
        }
    }
}

void OLED_Scroll(uint8_t enable)
{
    static const uint8_t start[] = {0x2E, 0x26, 0x00, 0x00, 0x00,
                                    0x07, 0x00, 0xFF, 0x2F};
    static const uint8_t stop[] = {0x2E};
    OLED_Command(enable ? start : stop, enable ? sizeof(start) : sizeof(stop));
}
