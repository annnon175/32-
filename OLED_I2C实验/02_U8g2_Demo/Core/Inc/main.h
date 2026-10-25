#ifndef MAIN_H
#define MAIN_H
#include "stm32f1xx_hal.h"
extern I2C_HandleTypeDef hi2c1;
extern UART_HandleTypeDef huart1;
void Board_Init(void);
void Error_Handler(void);
#endif
