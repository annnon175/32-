#ifndef LAB_CONFIG_H
#define LAB_CONFIG_H
#define UART_BAUD 115200U /* 与串口助手保持一致。 */
#define TX_DELAY_MS 0U   /* 改成 50U，可比较有无发送间隔。 */
#define UART_TIMEOUT_MS 200U /* 256 字节在 115200 下约需 22 ms。 */
#endif
