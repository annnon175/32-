#include "main.h"
#include "lab_config.h"
#include "oled.h"
#include "aht20.h"
#include "demo.h"
#include "gfx.h"

/* Keil Watch里看这几个变量，老师问时从主循环讲起。 */
volatile uint8_t g_page, g_oled_ready, g_auto_mode;
volatile uint32_t g_frames;

int main(void)
{
    uint32_t now, last_frame, page_start, retry_at, page_duration, scroll_duration;
    uint8_t command, previous_page = 0, page_index = 0;
    DemoData data;
#ifdef USE_U8G2
    static const uint8_t pages[] = {7, 1, 6, 3, 4, 5};
#else
    static const uint8_t pages[] = {1, 3, 5};
#endif
    static uint8_t help[] = "OLED demo: 0=auto, 1=name, 2=AHT20, 3=scroll X, "
                            "4=scroll Y, 5=HW scroll, 6=graphics, 7=avatar\r\n";
    Board_Init();
    HAL_Delay(100);
    g_oled_ready = OLED_Probe();
    Gfx_Init();
    OLED_Scroll(0); /* MCU复位不一定让OLED断电，先明确停止旧的硬件滚屏。 */
    scroll_duration = (uint32_t)(Gfx_TextWidth(SCROLL_TEXT) + 144) * 40U + 1000U;
#if ENABLE_AHT20
    AHT20_Init();
#endif
    HAL_UART_Transmit(&huart1, help, sizeof(help) - 1, 100);
    g_auto_mode = START_PAGE == 0;
    g_page = g_auto_mode ? pages[0] : START_PAGE;
    last_frame = page_start = retry_at = HAL_GetTick();

    while (1) {
        now = HAL_GetTick();
        /* 先收选页命令，再更新传感器，最后刷新画面。无需额外按键。 */
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_ORE)) __HAL_UART_CLEAR_OREFLAG(&huart1);
        if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) &&
            HAL_UART_Receive(&huart1, &command, 1, 1) == HAL_OK) {
            if (command == '0') {
                g_auto_mode = 1; page_index = 0; g_page = pages[0]; page_start = now;
            } else if (command >= '1' && command <= '7') {
                g_auto_mode = 0; g_page = command - '0'; page_start = now;
            } else if (command == '?') HAL_UART_Transmit(&huart1, help, sizeof(help) - 1, 100);
        }
        page_duration = g_page == 3 ? scroll_duration : PAGE_INTERVAL_MS;
        if (g_page == 4) page_duration = 10000U;
        if (g_auto_mode && now - page_start >= page_duration) {
            page_index = (uint8_t)((page_index + 1) % sizeof(pages));
            g_page = pages[page_index];
            page_start = now;
        }
#if ENABLE_AHT20
        AHT20_Task(now);
#endif
        if (!g_oled_ready && now - retry_at >= 2000U) {
            retry_at = now;
            g_oled_ready = OLED_Probe();
            if (g_oled_ready) { Gfx_Init(); previous_page = 0; }
        }
        if (!g_oled_ready || now - last_frame < FRAME_INTERVAL_MS) continue;
        last_frame = now;
        if (g_page != previous_page) OLED_Scroll(0);
        /* 硬件滚屏页只上传一次显存，滚动交给SSD1306。 */
        if (g_page == 5 && previous_page == 5) continue;
        data.temperature_x10 = g_aht20.temperature_x10;
        data.humidity_x10 = g_aht20.humidity_x10;
        data.sensor_valid = g_aht20.valid;
        data.sample_age_ms = now - g_aht20.last_ok;
        Demo_Draw(g_page, now - page_start, &data);
        Gfx_Send();
        if (g_page == 5) OLED_Scroll(1);
        previous_page = g_page;
        ++g_frames;
    }
}
