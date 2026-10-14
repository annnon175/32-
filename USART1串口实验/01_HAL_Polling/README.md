# 01_HAL_Polling

STM32F103C8T6，HSI / 2 × 16 = 64 MHz；USART1：PA9 TX、PA10 RX，115200 8N1。调试器沿用 CMSIS-DAP。

打开 `01_HAL_Polling.uvprojx` 编译；备用构建：PowerShell 执行 `./build.ps1`。

`Core/Inc/lab_config.h` 的 `TX_DELAY_MS=0` 默认无额外延时，改为 50 可做对照；`UART_BAUD` 可改 460800，电脑需同步设置。

发送 `hello windows!\r\n`；电脑发 ASCII `#` 暂停、`*` 恢复，别启用 HEX 文本发送。轮询版可能在一行中间暂停并原位继续；中断版完成当前16字节行后暂停，DMA版完成当前256字节块后暂停。

STM32 PA9 -> 模块 RX，PA10 <- 模块 TX，GND-GND。只用适配 3.3V IO 的 TTL/CMOS 侧，RS232侧不能直连 MCU。旧实验 PA9/PA10 的 LED 负载先移除。板与模块各自供电时不用跨接 VCC。

Watch：`g_running`、`g_tx_lines`、`g_rx_bytes`、`g_uart_errors`、`g_fatal_error`。调试暂停会影响在线通信，不用于测速。

HAL/CMSIS 来自用户旧工程。许可文件见 Drivers 下相应目录；保留上游版权。

配套 .ioc 用于在 CubeMX 查看/重建配置。重新生成时另选目录，保留当前已经编译的 main.c；它不是仅靠配置就能生成 #/* 业务逻辑的文件。

