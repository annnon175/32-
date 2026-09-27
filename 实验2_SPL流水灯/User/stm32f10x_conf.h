#ifndef __STM32F10X_CONF_H
#define __STM32F10X_CONF_H

/* 本实验只启用 GPIO 与 RCC 两个标准外设库模块。 */
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);
#else
#define assert_param(expr) ((void)0U)
#endif

#endif

