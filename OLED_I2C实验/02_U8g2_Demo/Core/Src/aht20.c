#include "main.h"
#include "aht20.h"
#include "lab_config.h"

AHT20_Data g_aht20;
volatile uint32_t g_sensor_errors;
static uint32_t next_start, conversion_start;
static uint8_t measuring;

static uint8_t crc8(const uint8_t *data, uint8_t count)
{
    uint8_t crc = 0xFF, i, bit;
    for (i = 0; i < count; ++i) {
        crc ^= data[i];
        for (bit = 0; bit < 8; ++bit)
            crc = (uint8_t)((crc & 0x80) ? (crc << 1) ^ 0x31 : crc << 1);
    }
    return crc;
}

uint8_t AHT20_Decode(const uint8_t raw[7], int32_t *t, uint16_t *rh)
{
    uint32_t hraw, traw;
    if ((raw[0] & 0x80) || !(raw[0] & 0x08) || crc8(raw, 6) != raw[6]) return 0;
    hraw = ((uint32_t)raw[1] << 12) | ((uint32_t)raw[2] << 4) | (raw[3] >> 4);
    traw = ((uint32_t)(raw[3] & 0x0F) << 16) | ((uint32_t)raw[4] << 8) | raw[5];
    *rh = (uint16_t)((hraw * 1000U + 524288U) >> 20);
    *t = (int32_t)((traw * 2000U + 524288U) >> 20) - 500;
    return 1;
}

static uint8_t write_cmd(uint8_t *cmd, uint16_t count)
{
    return HAL_I2C_Master_Transmit(&hi2c1, AHT20_ADDR << 1, cmd,
                                   count, I2C_TIMEOUT_MS) == HAL_OK;
}

void AHT20_Init(void)
{
    uint8_t status = 0, cmd[] = {0xBE, 0x08, 0x00};
    HAL_Delay(100);
    if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDR << 1, &status, 1,
                              I2C_TIMEOUT_MS) == HAL_OK && !(status & 0x08)) {
        (void)write_cmd(cmd, sizeof(cmd));
        HAL_Delay(10);
    }
    next_start = HAL_GetTick();
}

void AHT20_Task(uint32_t now)
{
    uint8_t raw[7], cmd[] = {0xAC, 0x33, 0x00};
    if (!measuring) {
        if ((int32_t)(now - next_start) < 0) return;
        next_start = now + SENSOR_INTERVAL_MS;
        if (!write_cmd(cmd, sizeof(cmd))) { ++g_sensor_errors; return; }
        conversion_start = now;
        measuring = 1;
        return;
    }
    /* 等待转换时主循环继续画图，不用HAL_Delay(80)卡住动画。 */
    if (now - conversion_start < 85U) return;
    if (HAL_I2C_Master_Receive(&hi2c1, AHT20_ADDR << 1, raw, 7,
                              I2C_TIMEOUT_MS) != HAL_OK) {
        ++g_sensor_errors;
        measuring = 0;
        return;
    }
    if ((raw[0] & 0x80) && now - conversion_start < 200U) return;
    measuring = 0;
    if (AHT20_Decode(raw, &g_aht20.temperature_x10, &g_aht20.humidity_x10)) {
        g_aht20.last_ok = now;
        g_aht20.valid = 1;
    } else ++g_sensor_errors;
}
