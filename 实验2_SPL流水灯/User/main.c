#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/*
 * 实验 2：STM32F10x 标准外设库三灯流水
 * 芯片：STM32F103C8T6，复位后使用内部 HSI 8 MHz。
 * 三只外接 LED：PA8、PA9、PA10，高电平点亮，统一使用 GPIOA。
 */

#define DELAY_LOOP_COUNT  888889UL

/* Keil 启动文件会调用 SystemInit。本实验保持复位默认的 HSI 8 MHz。 */
uint32_t SystemCoreClock = 8000000UL;

void SystemInit(void)
{
}

void SystemCoreClockUpdate(void)
{
    SystemCoreClock = 8000000UL;
}

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t active_low;
} led_t;

static const led_t g_leds[] = {
    {GPIOA, GPIO_Pin_8,  0U},
    {GPIOA, GPIO_Pin_9,  0U},
    {GPIOA, GPIO_Pin_10, 0U}
};

#define LED_COUNT ((uint32_t)(sizeof(g_leds) / sizeof(g_leds[0])))

/*
 * 给 Keil Logic Analyzer 使用的三个辅助标量，记录对应 GPIO 的输出电平，
 * 便于分别画出三条数字波形；g_logic_step 用来标记换灯次数。
 */
volatile uint32_t g_logic_pa8 = 0UL;
volatile uint32_t g_logic_pa9 = 0UL;
volatile uint32_t g_logic_pa10 = 0UL;
volatile uint32_t g_logic_step = 0UL;

/* 本实验未使用 SysTick；保留空处理函数以匹配启动文件的向量表。 */
void SysTick_Handler(void)
{
}

static void delay_loop_approximately_1s(void)
{
    volatile uint32_t count;

    /*
     * 这是实验要求的软件循环延时。循环体时间会受编译器版本、优化等级
     * 和指令排布影响，所以 DELAY_LOOP_COUNT 需要结合 Keil Logic Analyzer
     * 的时间轴校准，不能把它当作精密时基。
     */
    for (count = 0UL; count < DELAY_LOOP_COUNT; ++count) {
        __NOP();
    }
}

static void led_write(const led_t *led, uint8_t on)
{
    BitAction level;

    /* active_low=1 时，写低电平才是点亮。 */
    if ((on ^ led->active_low) != 0U) {
        level = Bit_SET;
    } else {
        level = Bit_RESET;
    }
    GPIO_WriteBit(led->port, led->pin, level);
}

static void all_leds_off(void)
{
    uint32_t index;

    for (index = 0UL; index < LED_COUNT; ++index) {
        led_write(&g_leds[index], 0U);
    }
}

static void logic_trace_update(uint32_t active_index)
{
    g_logic_pa8 = (active_index == 0UL) ? 1UL : 0UL;
    g_logic_pa9 = (active_index == 1UL) ? 1UL : 0UL;
    g_logic_pa10 = (active_index == 2UL) ? 1UL : 0UL;
    g_logic_step++;
}

static void gpio_init(void)
{
    GPIO_InitTypeDef gpio;

    /* 三只灯都在 GPIOA，只需打开 GPIOA 时钟。 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* 先写入熄灭电平，随后切换输出模式，减少上电瞬间闪烁。 */
    GPIO_ResetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10);

    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;

    gpio.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10;
    GPIO_Init(GPIOA, &gpio);

    all_leds_off();
}

int main(void)
{
    uint32_t index;

    /* 复位默认 HSI=8 MHz，本实验不启用外部晶振和 PLL。 */
    gpio_init();

    for (;;) {
        for (index = 0UL; index < LED_COUNT; ++index) {
            all_leds_off();
            led_write(&g_leds[index], 1U);
            logic_trace_update(index);
            delay_loop_approximately_1s();
        }
    }
}
