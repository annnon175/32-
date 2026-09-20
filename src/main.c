#include <stdint.h>

/*
 * STM32F103C8T6 (Blue Pill) 四灯流水灯
 *
 * 外接 LED：PA8、PA15、PB8，默认高电平点亮。
 * 板载 LED：PC13，Blue Pill 原理图决定其低电平点亮。
 *
 * 本文件只用 C 语言读写寄存器，不调用 HAL、LL 或标准外设库。
 * 编译参数 INCLUDE_ONBOARD_LED=0 生成三灯版，=1 生成四灯版。
 */

#ifndef INCLUDE_ONBOARD_LED
#define INCLUDE_ONBOARD_LED 1
#endif

/* 若外接 LED 的正极接 3.3 V、负极接 GPIO，应改为 1（低电平点亮）。 */
#ifndef EXTERNAL_LEDS_ACTIVE_LOW
#define EXTERNAL_LEDS_ACTIVE_LOW 0
#endif

#define BIT(n) (1UL << (n))
#define REG32(address) (*(volatile uint32_t *)(address))

/* RCC：复位与时钟控制器，基地址 0x40021000。 */
#define RCC_CR          REG32(0x40021000UL)
#define RCC_CFGR        REG32(0x40021004UL)
#define RCC_APB2ENR     REG32(0x40021018UL)

/* AFIO：复用功能 I/O，MAPR 用于关闭 JTAG 并保留 SWD。 */
#define AFIO_MAPR       REG32(0x40010004UL)

/* GPIO 端口基地址。 */
#define GPIOA_BASE      0x40010800UL
#define GPIOB_BASE      0x40010C00UL
#define GPIOC_BASE      0x40011000UL

/* GPIO 寄存器相对端口基地址的偏移量。 */
#define GPIO_CRL(base)  REG32((base) + 0x00UL)
#define GPIO_CRH(base)  REG32((base) + 0x04UL)
#define GPIO_IDR(base)  REG32((base) + 0x08UL)
#define GPIO_ODR(base)  REG32((base) + 0x0CUL)
#define GPIO_BSRR(base) REG32((base) + 0x10UL)
#define GPIO_BRR(base)  REG32((base) + 0x14UL)
#define GPIO_LCKR(base) REG32((base) + 0x18UL)

/* Cortex-M3 SysTick 寄存器。 */
#define SYST_CSR        REG32(0xE000E010UL)
#define SYST_RVR        REG32(0xE000E014UL)
#define SYST_CVR        REG32(0xE000E018UL)

#define SYSTEM_CORE_CLOCK_HZ 8000000UL
#define SYSTICK_HZ            1000UL

typedef struct {
    uint32_t port_base;
    uint8_t pin;
    uint8_t active_low;
} led_t;

/* 中断服务程序与主循环共享，必须使用 volatile。 */
static volatile uint32_t g_milliseconds = 0UL;

/* 三只外接灯：默认 GPIO 高电平点亮。 */
static const led_t g_external_leds[] = {
    {GPIOA_BASE, 8U,  EXTERNAL_LEDS_ACTIVE_LOW},
    {GPIOA_BASE, 15U, EXTERNAL_LEDS_ACTIVE_LOW},
    {GPIOB_BASE, 8U,  EXTERNAL_LEDS_ACTIVE_LOW}
};

/* Blue Pill 板载 PC13 LED：3.3 V -> LED -> 电阻 -> PC13，低电平点亮。 */
static const led_t g_onboard_led = {GPIOC_BASE, 13U, 1U};

void SysTick_Handler(void)
{
    g_milliseconds++;
}

static void clock_use_hsi_8mhz(void)
{
    /* HSION=1，打开内部 8 MHz RC 振荡器。 */
    RCC_CR |= BIT(0);
    while ((RCC_CR & BIT(1)) == 0UL) {
        /* 等待 HSIRDY=1。 */
    }

    /* SW[1:0]=00，选择 HSI 为系统时钟。 */
    RCC_CFGR &= ~0x3UL;
    while ((RCC_CFGR & (0x3UL << 2U)) != 0UL) {
        /* 等待 SWS[1:0]=00，确认系统时钟已切换到 HSI。 */
    }
}

static void systick_init_1ms(void)
{
    /* 8 000 000 / 1 000 = 8 000 个时钟周期；LOAD 写 N-1。 */
    SYST_RVR = (SYSTEM_CORE_CLOCK_HZ / SYSTICK_HZ) - 1UL;
    SYST_CVR = 0UL;

    /* CLKSOURCE=1 使用处理器时钟；TICKINT=1 开中断；ENABLE=1 启动。 */
    SYST_CSR = BIT(2) | BIT(1) | BIT(0);
}

static void delay_ms(uint32_t milliseconds)
{
    const uint32_t start = g_milliseconds;

    /* 无符号减法可正确处理 32 位毫秒计数器回绕。 */
    while ((uint32_t)(g_milliseconds - start) < milliseconds) {
        /* 忙等待；SysTick 中断仍会每 1 ms 更新计数。 */
    }
}

static void gpio_config_output_pp_2mhz(uint32_t port_base, uint8_t pin)
{
    volatile uint32_t *config_register;
    uint32_t shift;
    uint32_t value;

    /* F1 每个引脚占 4 位：CNF[1:0] + MODE[1:0]。
     * 0b0010 = CNF=00（通用推挽输出），MODE=10（最大 2 MHz）。
     */
    if (pin < 8U) {
        config_register = (volatile uint32_t *)(port_base + 0x00UL);
        shift = (uint32_t)pin * 4UL;
    } else {
        config_register = (volatile uint32_t *)(port_base + 0x04UL);
        shift = ((uint32_t)pin - 8UL) * 4UL;
    }

    value = *config_register;
    value &= ~(0xFUL << shift);
    value |=  (0x2UL << shift);
    *config_register = value;
}

static void gpio_set_high(const led_t *led)
{
    /* BSRR 低 16 位写 1：对应引脚原子置位为高电平。 */
    GPIO_BSRR(led->port_base) = BIT(led->pin);
}

static void gpio_set_low(const led_t *led)
{
    /* BRR 低 16 位写 1：对应引脚原子复位为低电平。 */
    GPIO_BRR(led->port_base) = BIT(led->pin);
}

static void led_set(const led_t *led, uint8_t on)
{
    const uint8_t output_high = (uint8_t)(on ^ led->active_low);

    if (output_high != 0U) {
        gpio_set_high(led);
    } else {
        gpio_set_low(led);
    }
}

static void all_leds_off(void)
{
    uint32_t index;

    for (index = 0UL; index < 3UL; ++index) {
        led_set(&g_external_leds[index], 0U);
    }
    led_set(&g_onboard_led, 0U);
}

static void gpio_init(void)
{
    /* APB2ENR：AFIOEN(bit0)、IOPAEN(bit2)、IOPBEN(bit3)、IOPCEN(bit4)。 */
    RCC_APB2ENR |= BIT(0) | BIT(2) | BIT(3) | BIT(4);

    /* PA15 上电默认属于 JTAG 的 JTDI。
     * SWJ_CFG=010：关闭 JTAG-DP，释放 PA15/PB3/PB4；保留 PA13/PA14 的 SWD。
     * 这样 ST-Link 仍可通过 SWDIO/SWCLK 下载和调试。
     */
    AFIO_MAPR = (AFIO_MAPR & ~(0x7UL << 24U)) | (0x2UL << 24U);

    /* 先写“熄灭”电平到输出锁存器，再切换为输出，避免初始化闪烁。 */
    all_leds_off();

    gpio_config_output_pp_2mhz(GPIOA_BASE, 8U);
    gpio_config_output_pp_2mhz(GPIOA_BASE, 15U);
    gpio_config_output_pp_2mhz(GPIOB_BASE, 8U);
    gpio_config_output_pp_2mhz(GPIOC_BASE, 13U);
}

int main(void)
{
    uint32_t index;

    clock_use_hsi_8mhz();
    gpio_init();
    systick_init_1ms();

    for (;;) {
        /* 三只外接 LED 依次点亮，每灯保持 1 秒。 */
        for (index = 0UL; index < 3UL; ++index) {
            all_leds_off();
            led_set(&g_external_leds[index], 1U);
            delay_ms(1000UL);
        }

#if INCLUDE_ONBOARD_LED
        /* 四灯版追加板载 PC13；低电平点亮逻辑已由 led_set 自动处理。 */
        all_leds_off();
        led_set(&g_onboard_led, 1U);
        delay_ms(1000UL);
#endif
    }
}

