# STM32F103C8T6 实验工程

这里放STM32课程实验代码，包括流水灯、外部中断、USART1串口、TIM2定时器/PWM，以及OLED与U8g2显示。各实验目录保留可直接打开的Keil工程和依赖库。

| 实验 | 工程位置 | LED 接线 |
|---|---|---|
| 1：寄存器 | 仓库根目录的 `src/main.c`、`Makefile` | PA8、PA15、PB8、板载 PC13 |
| 2：标准外设库 | `实验2_SPL流水灯/实验2_SPL流水灯.uvprojx` | PA8、PA9、PA10 |
| 3：HAL + 外部中断 | `实验3_HAL按键中断流水灯/实验3_HAL按键中断流水灯.uvprojx` | PA8、PA9、PA10；PA0 短接 GND 一下模拟按键 |


## USART1串口实验

串口代码在 [USART1串口实验](USART1串口实验/README.md)，包含HAL轮询、中断和DMA三个独立Keil工程。
默认115200、8N1，PA9=TX、PA10=RX，发送`hello windows!`，`#`暂停、`*`恢复。
三个115200版本和DMA460800已通过真实C8T6/DAP串口测试；具体打开方法、参数与验证范围见该目录README。

## TIM2定时器与PWM实验

工程在 [TIM2定时器PWM实验](TIM2定时器PWM实验/README.md)。含定时器中断闪烁和PWM呼吸灯两个独立HAL工程：PA0控制外接LED，完整亮灭周期2s；PWM为1kHz，呼吸周期约1.6s。
GCC和Keil均编译通过，CubeMX配置已读入保存。两个固件已真实烧录、回读一致，用户确认LED闪烁和呼吸正常，Keil已观察到PA0 PWM仿真方波。

## OLED与U8g2显示实验

代码在 [OLED_I2C实验](OLED_I2C实验/README.md)，包含基础OLED和U8g2两个独立HAL工程。SSD1306四针屏连接PB6/SCL、PB7/SDA、3.3V和GND。

显示姓名学号、中文长句、左右/上下滚动、硬件滚屏、基本图形及卡通头像；USART1可按文本数字选页，发送`0`恢复自动轮播。默认关闭AHT20扩展。打开路径、串口命令及代码入口见该目录README。

## 实验一引脚

| 顺序 | LED | 引脚 | 默认有效电平 |
|---:|---|---|---|
| 1 | 外接 LED 1 | PA8 | 高 |
| 2 | 外接 LED 2 | PA15 | 高 |
| 3 | 外接 LED 3 | PB8 | 高 |
| 4 | Blue Pill 板载 LED | PC13 | 低 |

注意区分 `PA15` 和 `PA13`。PA13/PA14 是 SWD 下载接口，程序不会把它们改成普通 GPIO。PA15 默认是 JTAG JTDI，实验一的程序仅关闭 JTAG、保留 SWD。

## 打开和构建

实验二、三分别打开上表中的 `.uvprojx`，在 Keil 中 Rebuild。工程带有使用到的 SPL/HAL 源码、CMSIS 头文件和启动文件；烧录调试器选 CMSIS-DAP（DAPLink）。两个子文件夹也各有 `build.ps1` 和 `Makefile`，供 GNU Arm Toolchain 构建。

实验一的 GNU Arm 构建方法如下。Windows PowerShell：

Windows PowerShell：

```powershell
.\build.ps1
```

生成：

- `build/three-led/bluepill-led-three-led.hex`：三只外接灯版。
- `build/four-led/bluepill-led-four-led.hex`：三只外接灯加 PC13 的提交版。
- 同目录还会生成 ELF、BIN 和 MAP 文件。

Linux/macOS（已安装 GNU Make 与 Arm GNU Toolchain）：

```bash
make
```

## 烧录

DAPLink 与开发板连接：GND、SWDIO（PA13）、SWCLK（PA14），供电及 3.3 V/Vref 按实际 DAPLink 模块接法连接。BOOT0 保持从 Flash 启动。实验一的四灯版 HEX 可写入地址 `0x08000000`；实验二、三可直接在 Keil 中用 CMSIS-DAP 下载。

## 外接 LED 极性

默认接法为：

```text
GPIO ---- 330 Ω～1 kΩ ---- LED 正极(长脚)
GND  -------------------- LED 负极(短脚/平边)
```

若实物接成 `3.3V -> 电阻 -> LED -> GPIO`，请在编译时定义：

```text
EXTERNAL_LEDS_ACTIVE_LOW=1
```

限流电阻属于 LED 的电气保护要求。题目未写不代表可以省略；无电阻直连不作为安全接法或实物验收结论。


## 验证范围

- 实验一已用 CMSIS-DAP 烧录四灯版固件，完成 Flash 回读哈希校验。
- 实验二、三的 Keil 工程已编译通过；实验二的软件仿真显示三灯状态依次切换，平均换灯间隔约 0.889 s。软件仿真不是物理管脚测量。
- 实验三烧录后复位，板上三只外接 LED 正常流水；没有独立按键时，PA0 短接 GND 一下后松开即可触发下降沿。
