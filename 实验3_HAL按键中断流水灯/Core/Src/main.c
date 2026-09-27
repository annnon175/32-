#include "main.h"

/*
 * 实验 3：HAL 库 + PA0 外部中断控制三灯流水暂停/继续
 * 三只外接 LED：PA8、PA9、PA10，统一使用 GPIOA，高电平点亮。
 * 按键可用导线代替：PA0 短接 GND 一下，内部上拉，下降沿触发。
 */

#define LED_STEP_MS       1000UL
#define BUTTON_DEBOUNCE_MS 50UL

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t active_low;
} led_t;

static const led_t g_leds[] = {
    {GPIOA, GPIO_PIN_8,  0U},
    {GPIOA, GPIO_PIN_9,  0U},
    {GPIOA, GPIO_PIN_10, 0U}
};

#define LED_COUNT ((uint32_t)(sizeof(g_leds) / sizeof(g_leds[0])))

/* 主循环与中断共同访问，必须声明为 volatile。 */
static volatile uint8_t g_running = 1U;
/* 允许上电后的第一次按键立即生效。 */
static volatile uint32_t g_last_button_tick = 0UL - BUTTON_DEBOUNCE_MS;

static void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void led_write(const led_t *led, uint8_t on);
static void all_leds_off(void);
static void show_led(uint32_t index);

/* GNU 裸机链接时由启动文件调用 __libc_init_array，提供空的初始化钩子。 */
void _init(void)
{
}

void _fini(void)
{
}

int main(void)
{
    uint32_t led_index = 0UL;
    uint32_t last_step_tick;
    uint8_t was_running;

    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();

    show_led(led_index);
    last_step_tick = HAL_GetTick();
    was_running = g_running;

    for (;;) {
        const uint32_t now = HAL_GetTick();

        /* 恢复运行时重新开始本灯的 1 秒计时，不会刚恢复就立刻跳灯。 */
        if ((was_running == 0U) && (g_running != 0U)) {
            last_step_tick = now;
        }
        was_running = g_running;

        /* 非阻塞节拍：暂停时保留当前灯，恢复后从当前灯继续。 */
        if ((g_running != 0U) && ((uint32_t)(now - last_step_tick) >= LED_STEP_MS)) {
            last_step_tick = now;
            led_index = (led_index + 1UL) % LED_COUNT;
            show_led(led_index);
        }
    }
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    /* 使用内部 HSI 8 MHz，不依赖板上外部晶振。 */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV1;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* 先写入熄灭电平。 */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10, GPIO_PIN_RESET);

    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;

    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
    HAL_GPIO_Init(GPIOA, &gpio);

    /* PA0 内部上拉，按键接地，按下产生下降沿中断。 */
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_IT_FALLING;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);

    HAL_NVIC_SetPriority(EXTI0_IRQn, 2U, 0U);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);

    all_leds_off();
}

static void led_write(const led_t *led, uint8_t on)
{
    GPIO_PinState level;

    level = ((on ^ led->active_low) != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(led->port, led->pin, level);
}

static void all_leds_off(void)
{
    uint32_t index;

    for (index = 0UL; index < LED_COUNT; ++index) {
        led_write(&g_leds[index], 0U);
    }
}

static void show_led(uint32_t index)
{
    all_leds_off();
    led_write(&g_leds[index], 1U);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_0) {
        const uint32_t now = HAL_GetTick();

        /* 简单去抖。中断内不延时，只切换状态。 */
        if ((uint32_t)(now - g_last_button_tick) >= BUTTON_DEBOUNCE_MS) {
            g_last_button_tick = now;
            g_running ^= 1U;
        }
    }
}

void Error_Handler(void)
{
    __disable_irq();
    all_leds_off();
    for (;;) {
        /* 时钟配置失败时停在此处，便于调试定位。 */
    }
}
