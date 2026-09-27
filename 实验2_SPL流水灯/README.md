# 实验 2：标准外设库流水灯

实验二只使用 GPIOA：PA8、PA9、PA10 分别接三只外接 LED，高电平点亮；不使用板载 PC13 灯。

工程内只加入本实验使用的两个标准外设库源文件：

- `stm32f10x_gpio.c/.h`
- `stm32f10x_rcc.c/.h`

同时包含 `stm32f10x.h`、CMSIS Core 头文件、启动文件和链接脚本，因此可直接用 `build.ps1` 编译。

软件循环延时并不是精密时基。默认 `DELAY_LOOP_COUNT=888889`；依据本工程 Keil Arm Compiler 5 的反汇编和 Cortex-M3 指令周期表，单步延时估算约为 0.89～1.11 秒。实际时间应在 Keil 软件仿真的 Logic Analyzer 时间轴上测量后再微调。三灯流水时，同一只灯再次点亮的周期约为 2.67～3.33 秒。
