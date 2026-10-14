# STM32F103C8T6 定时器与PWM实验

两个独立HAL工程，使用TIM2和PA0。板子默认内部HSI时钟，外部晶振不参与运行。

| 工程 | 功能 | Keil入口 |
| --- | --- | --- |
| 01_TIM2_Blink | 亮1秒、灭1秒；完整周期2秒 | [打开工程](01_TIM2_Blink/01_TIM2_Blink.uvprojx) |
| 02_TIM2_PWM | 1kHz PWM，呼吸周期约1.6秒 | [打开工程](02_TIM2_PWM/02_TIM2_PWM.uvprojx) |

PA0 → 330Ω电阻 → LED长脚；LED短脚 → GND。板载PC13灯没有TIM2 PWM复用，不用它代替本实验LED。

HSI 8MHz / 2 × 16 = 64MHz；APB1=32MHz，因APB1分频不为1，TIM2输入时钟=64MHz。
闪烁PSC=63999、ARR=999，每1秒进入TIM2中断翻转GPIO；PWM PSC=63、ARR=999，频率1000Hz。
PWM比较值范围0~1000，ARR=999时CCR=1000是100%高电平。呼吸曲线每4ms更新一步，200步升、200步降。

Keil：ARM Compiler 5.06，默认CMSIS-DAP下载。GCC：在项目目录运行 `./build.ps1`。
包含HAL/CMSIS依赖，HAL版本1.1.10；厂商许可证保留在Drivers目录。
IOC已通过CubeMX 6.18.1读入和保存。若重新生成，先选另一个目录，再移入main.c的业务代码。

两工程GCC和Keil均编译通过（0错误、0警告）。2026-10-14两个固件已真实烧录并Flash回读一致。用户目视确认：亮约1秒、灭约1秒；呼吸灯正常渐亮渐暗约1～2秒一轮。PWM运行中采样CCR1=0～1000且计数器变化。目视和寄存器记录不代替精确PWM波形读数。

## Keil逻辑分析仪

打开相应 `_Simulator.uvprojx`，按Ctrl+F5进入软件仿真，View → Analysis Windows → Logic Analyzer。
Setup添加 `PORTA & 0x0001`，Display Type选Bit。闪烁观察2s周期；PWM调整到毫秒尺度，观察约1ms周期。
仿真副本选择STM32F103C8外设模型（DARMSTM.DLL）；需本机Keil带该模型。该配置尚未实际运行验证。
若没有PORTA虚拟寄存器或TIM2模型，不要把 `g_ccr`、`g_duty`画出的曲线当PWM引脚波形。
需检查Options for Target → Debug → Use Simulator，并确认器件模拟支持；报告中保留实际观察后的填写位置。

PWM工程 `Core/Inc/lab_config.h` 的 `PWM_FIXED_PERCENT`：-1为呼吸，0~100为固定占空比。
固定50%时理论周期1ms、高电平0.5ms；测试后改回-1重新编译。

## 参考

- [ST HAL PWMOutput示例](https://github.com/STMicroelectronics/STM32CubeF1/tree/master/Projects/STM3210E_EVAL/Examples/TIM/TIM_PWMOutput)
- [STM32F103C8数据手册](https://www.st.com/resource/en/datasheet/stm32f103c8.pdf)
- [课程参考博客](https://blog.csdn.net/Morzart/article/details/134123197)
- [Keil TIM2软件仿真说明](https://www.keil.com/dd/vtr/4235/9060.htm)
- [Keil PORTA虚拟寄存器说明](https://www.keil.com/dd/vtr/4235/9048.htm)
