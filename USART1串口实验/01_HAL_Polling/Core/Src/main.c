#include "main.h"
#include "lab_config.h"

UART_HandleTypeDef huart1;
/* g_running: 1继续发，0暂停；其余 g_ 变量是计数/错误标记，可在 Watch 里看。 */
volatile uint32_t g_running = 1U;
volatile uint32_t g_tx_lines = 0U;
volatile uint32_t g_rx_bytes = 0U;
volatile uint32_t g_uart_errors = 0U;
volatile uint32_t g_fatal_error = 0U;
static uint8_t rx_byte;
static uint8_t hello[] = "hello windows!\r\n";
/* sizeof 会算上字符串结尾的 \0；串口不发送那个 \0。 */
#define LINE_BYTES (sizeof(hello) - 1U)

static void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
/* 默认 TX_DELAY_MS=0，发完一行就发下一行；改成50可做延时对照。 */
static uint32_t tx_due(uint32_t last)
{
#if TX_DELAY_MS > 0U
    return (uint32_t)(HAL_GetTick() - last) >= TX_DELAY_MS;
#else
    (void)last;
    return 1U;
#endif
}
static void command(uint8_t ch)
{
    ++g_rx_bytes;
    if (ch == '#') {
        g_running = 0U;
    } else if (ch == '*') {
        g_running = 1U;
    }
}

/* GCC 启动文件需要的初始化钩子。 */
void _init(void) {}
void _fini(void) {}

int main(void)
{
    uint32_t last_start = 0U;
    /* 轮询版一次只发1字节，pos记住当前发到hello的哪里。 */
    uint32_t pos = 0U;
    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();
    for (;;) {
        /* 第一件事：查接收。ORE表示来不及读旧字节，先清标志。 */
        /* 暂停时也要接收，不然收到 * 后就无法继续。 */
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE) != RESET) {
            __HAL_UART_CLEAR_OREFLAG(&huart1);
            ++g_uart_errors;
        }
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET) {
            if (HAL_UART_Receive(&huart1, &rx_byte, 1U, 0U) == HAL_OK) {
                command(rx_byte);
            } else {
                ++g_uart_errors;
            }
        }
        /* 第二件事：没暂停就发下一字节。HAL_UART_Transmit会等该字节发完。 */
        if (g_running && (pos || tx_due(last_start))) {
            if (HAL_UART_Transmit(&huart1, &hello[pos], 1U, UART_TIMEOUT_MS) == HAL_OK) {
                if (++pos == LINE_BYTES) {
                    pos = 0U;
                    ++g_tx_lines;
                    last_start = HAL_GetTick();
                }
            } else {
                ++g_uart_errors;
            }
        }
    }
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

static void MX_USART1_UART_Init(void)
{
    /* USART1: PA9发、PA10收；电脑端同样选115200、8N1。 */
    huart1.Instance = USART1;
    huart1.Init.BaudRate = UART_BAUD;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) Error_Handler();
}

void HAL_UART_MspInit(UART_HandleTypeDef *uart)
{
    GPIO_InitTypeDef gpio = {0};
    if (uart->Instance != USART1) return;
    /* HAL_UART_Init会走到这里，把PA9/PA10切到串口功能。 */
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpio);
    gpio.Pin = GPIO_PIN_10;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOA, &gpio);
}

void Error_Handler(void)
{
    g_fatal_error = 1U;
    __disable_irq();
    for (;;) {}
}
