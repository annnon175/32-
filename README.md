# STM32F103C8T6 Blue Pill 裸寄存器流水灯

本项目使用 STM32F103C8T6、面包板上的三只外接 LED 和板载 PC13 LED，实现间隔 1 秒的四灯流水效果。程序只使用 C 语言寄存器读写，不调用 HAL、LL 或标准外设库。

## 引脚

| 顺序 | LED | 引脚 | 默认有效电平 |
|---:|---|---|---|
| 1 | 外接 LED 1 | PA8 | 高 |
| 2 | 外接 LED 2 | PA15 | 高 |
| 3 | 外接 LED 3 | PB8 | 高 |
| 4 | Blue Pill 板载 LED | PC13 | 低 |

注意：照片中容易把 `PA15` 看成 `PA13`。PA13/PA14 是 SWD 下载接口，程序不会把它们改成普通 GPIO。PA15 默认是 JTAG JTDI，程序仅关闭 JTAG、保留 SWD。

## 构建

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

ST-Link 与开发板连接：`3.3V-3.3V`、`GND-GND`、`SWDIO-PA13`、`SWCLK-PA14`。可使用 STM32CubeProgrammer 将四灯版 HEX 写入地址 `0x08000000`。

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


## 实物验证

- 已通过 CMSIS-DAP 烧录四灯版固件，并完成 Flash 回读哈希校验。
- `assets/四灯流水实物验证.jpg` 汇总 PA8、PA15、PB8、PC13 四个点亮阶段。
- `assets/流水灯运行视频.mp4` 保存连续运行视频。
- 照片中的外接 LED 未串联限流电阻，因此只确认功能现象，不确认电气安全和长期可靠性。
