#include "main.h"
#include "lab_config.h"

TIM_HandleTypeDef htim2;
/* 这些变量方便在Keil Watch里看亮度变化，不代表引脚波形。 */
volatile uint32_t g_phase;
volatile uint32_t g_ccr;
volatile uint32_t g_duty;

static void SystemClock_Config(void);
static void TIM2_PWM_Init(void);

static void SetBrightness(uint32_t value)
{
    if (value > PWM_COUNTS) value = PWM_COUNTS;
    g_ccr = value;
    g_duty = value * 100 / PWM_COUNTS;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, value);
}

int main(void)
{
    uint32_t last_update;
    int direction = 1;
    HAL_Init();
    SystemClock_Config();
    TIM2_PWM_Init();
    SetBrightness(0);
    if (HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1) != HAL_OK) Error_Handler();

#if PWM_FIXED_PERCENT >= 0
    /* 测波形时固定占空比，平时设为-1就是呼吸灯。 */
    SetBrightness(PWM_COUNTS * PWM_FIXED_PERCENT / 100);
#endif
    last_update = HAL_GetTick();
    while (1) {
        if ((uint32_t)(HAL_GetTick() - last_update) >= BREATH_STEP_MS) {
            last_update += BREATH_STEP_MS;
#if PWM_FIXED_PERCENT < 0
            if (direction > 0) {
                ++g_phase;
                if (g_phase == BREATH_STEPS) direction = -1;
            } else {
                --g_phase;
                if (g_phase == 0) direction = 1;
            }
            /* 平方曲线：暗的时候变慢一些，视觉上比线性更平滑。 */
            SetBrightness(g_phase * g_phase * PWM_COUNTS /
                          (BREATH_STEPS * BREATH_STEPS));
#else
            (void)direction;
#endif
        }
    }
}

static void TIM2_PWM_Init(void)
{
    TIM_OC_InitTypeDef channel = {0};
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_TIM2_CLK_ENABLE();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = PWM_PSC;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = PWM_COUNTS - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) Error_Handler();
    channel.OCMode = TIM_OCMODE_PWM1;
    channel.Pulse = 0;
    channel.OCPolarity = TIM_OCPOLARITY_HIGH;
    channel.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &channel, TIM_CHANNEL_1) != HAL_OK)
        Error_Handler();
    /* PA0使用默认TIM2_CH1复用，不开重映射。 */
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
}

static void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};
    /* 使用内部时钟：8 MHz / 2 * 16 = 64 MHz。
       APB1 为 32 MHz，APB2 为 64 MHz。 */
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    osc.HSIState = RCC_HSI_ON;
    osc.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    osc.HSEState = RCC_HSE_OFF;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    osc.PLL.PLLMUL = RCC_PLL_MUL16;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) Error_Handler();
    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV2;
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2) != HAL_OK) Error_Handler();
}

void Error_Handler(void)
{
    /* 出错后停止计数，并把LED引脚拉低。 */
    GPIO_InitTypeDef gpio = {0};
    __HAL_RCC_GPIOA_CLK_ENABLE();
    if (htim2.Instance == TIM2) {
        TIM2->CCER = 0;
        TIM2->CR1 &= ~TIM_CR1_CEN;
        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0);
    }
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    gpio.Pin = GPIO_PIN_0;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &gpio);
    __disable_irq();
    while (1) {}
}

#if defined(__GNUC__)
/* GCC裸机链接不使用crt0，保留空初始化入口。 */
void _init(void) {}
#endif
