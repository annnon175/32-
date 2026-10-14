#include "main.h"
#include "lab_config.h"

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
volatile uint32_t g_running = 1U;
volatile uint32_t g_tx_busy = 0U;
volatile uint32_t g_tx_lines = 0U;
volatile uint32_t g_rx_bytes = 0U;
volatile uint32_t g_uart_errors = 0U;
volatile uint32_t g_fatal_error = 0U;
static volatile uint32_t rx_retry = 0U;
static uint8_t rx_byte;
static uint8_t hello[] = "hello windows!\r\n";
#define LINE_BYTES (sizeof(hello) - 1U)
#define DMA_LINES 16U
static uint8_t dma_data[LINE_BYTES * DMA_LINES];

static void SystemClock_Config(void);
static void MX_USART1_UART_Init(void);
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
    if (ch == '#') g_running = 0U;
    else if (ch == '*') g_running = 1U;
}

/* GCC 启动文件需要的初始化钩子。 */
void _init(void) {}
void _fini(void) {}

int main(void)
{
    uint32_t last_start = 0U;
    uint32_t tx_started = 0U;
    uint32_t i;
    HAL_Init();
    SystemClock_Config();
    MX_USART1_UART_Init();
    for (i = 0U; i < sizeof(dma_data); ++i) dma_data[i] = hello[i % LINE_BYTES];
    if (HAL_UART_Receive_IT(&huart1, &rx_byte, 1U) != HAL_OK) Error_Handler();
    for (;;) {
        if (rx_retry) {
            /* 接收出错后重新挂载，发送不受影响。 */
            uint32_t mask = __get_PRIMASK();
            __disable_irq();
            HAL_UART_AbortReceive(&huart1);
            __HAL_UART_CLEAR_OREFLAG(&huart1);
            rx_retry = (HAL_UART_Receive_IT(&huart1, &rx_byte, 1U) == HAL_OK) ? 0U : 1U;
            __set_PRIMASK(mask);
        }
        if (g_tx_busy && (uint32_t)(HAL_GetTick() - tx_started) >= UART_TIMEOUT_MS) {
            uint32_t mask = __get_PRIMASK();
            __disable_irq();
            HAL_UART_AbortTransmit(&huart1);
            g_tx_busy = 0U;
            ++g_uart_errors;
            __set_PRIMASK(mask);
        }
        if (g_running && !g_tx_busy && tx_due(last_start)) {
            HAL_StatusTypeDef status;
            uint32_t mask = __get_PRIMASK();
            __disable_irq();
            /* 启动前再看一次，避免刚收到 # 又发出下一组数据。 */
            if (g_running && !g_tx_busy) {
                g_tx_busy = 1U;
                tx_started = last_start = HAL_GetTick();
                status = HAL_UART_Transmit_DMA(&huart1, dma_data, (uint16_t)sizeof(dma_data));
                if (status != HAL_OK) {
                    g_tx_busy = 0U;
                    ++g_uart_errors;
                }
            }
            __set_PRIMASK(mask);
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
    __HAL_RCC_DMA1_CLK_ENABLE();
    hdma_usart1_tx.Instance = DMA1_Channel4;
    hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart1_tx.Init.Mode = DMA_NORMAL;
    hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) Error_Handler();
    __HAL_LINKDMA(uart, hdmatx, hdma_usart1_tx);
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    HAL_NVIC_SetPriority(USART1_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance != USART1) return;
    command(rx_byte);
    if (HAL_UART_Receive_IT(uart, &rx_byte, 1U) != HAL_OK) rx_retry = 1U;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance != USART1) return;
    g_tx_lines += DMA_LINES;
    g_tx_busy = 0U;
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    if (uart->Instance != USART1) return;
    ++g_uart_errors;
    rx_retry = 1U;
}

void Error_Handler(void)
{
    g_fatal_error = 1U;
    __disable_irq();
    for (;;) {}
}
