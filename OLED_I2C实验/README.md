# STM32F103C8T6 OLED与U8g2实验

四针SSD1306 I2C屏，128×64。包含两个可直接打开的HAL工程，推荐先运行U8g2演示工程。

| 工程 | 打开文件 | 内容 |
| --- | --- | --- |
| 基础OLED | `01_OLED_Basic/01_OLED_Basic.uvprojx` | 显存、中文点阵、文字和图形 |
| U8g2演示 | `02_U8g2_Demo/02_U8g2_Demo.uvprojx` | U8g2移植、姓名学号、左右/上下滚动、硬件滚屏、卡通头像 |

## 接线

| OLED | C8T6 |
| --- | --- |
| VCC | 3.3V |
| GND | GND |
| SCL | PB6 / I2C1_SCL |
| SDA | PB7 / I2C1_SDA |

按模块丝印接线。信号线需上拉到3.3V，常见OLED模块自带上拉。DAP使用PA13/SWDIO、PA14/SWCLK和GND，BOOT0保持0。

可选串口：PA9接DAP的RXD，PA10接DAP的TXD，共地，115200、8N1。

## 编译与演示

Keil打开上表的普通工程，使用ARM Compiler 5编译，CMSIS-DAP下载后按RST运行。配套`_Simulator.uvprojx`为仿真配置。两个工程均包含所需HAL/CMSIS源文件、启动文件和链接配置，U8g2工程保留裁剪后的图形库源码。

已安装GNU Arm Toolchain时，也可进入对应工程目录，在PowerShell执行：

```powershell
.\build.ps1
```

生成的HEX、BIN、ELF在`build`目录；编译输出不提交Git。

默认先显示头像约8秒，再自动切换姓名、图形、左右滚动、上下滚动和硬件滚屏。长句页按文字宽度延长停留时间，让整句滚完。

串口按ASCII文本发送单个字符，关闭HEX发送：

| 字符 | 页面 |
| --- | --- |
| `0` | 恢复自动轮播 |
| `1` | 姓名与学号 |
| `3` | 左右滚动 |
| `4` | 上下滚动 |
| `5` | SSD1306硬件滚屏 |
| `6` | U8g2图形与进度条 |
| `7` | 卡通头像 |
| `?` | 返回命令说明 |

手动选页会固定显示，发`0`恢复轮播。OLED程序不会连续输出hello，串口连接后可发`?`检查收发。

## 代码入口

- `Core/Inc/lab_config.h`：姓名学号、长句、地址、I2C速率、启动页。
- `Core/Src/main.c`：初始化、串口选页、自动轮播。
- `Core/Src/demo.c`：各页绘图和动画。
- `Core/Src/u8g2_port.c`：U8g2与HAL I2C字节回调。
- `Core/Src/oled.c`：SSD1306命令、显存发送和硬件滚屏。
- `Core/Src/avatar_bitmap.c`：两个64×64单色头像视图。

时钟使用HSI 8MHz÷2×16=64MHz，APB1为32MHz，I2C速率100kHz。OLED地址按7位`0x3C`配置，调用HAL/U8g2时左移为`0x78`。

当前`ENABLE_AHT20=0`，OLED独立演示。AHT20驱动作为扩展保留。`START_PAGE=0`为自动轮播。

## 检查记录与依赖

两个工程已通过Keil和GCC编译。U8g2程序已在C8T6上烧录、Flash回读校验，并确认卡通头像实物显示正常；姓名、中文滚动和图形有对应实拍记录。I2C理论图与离线绘图预览不作为示波器实测。

U8g2固定提交与字体许可证见[依赖来源.md](依赖来源.md)和`字库源文件`。报告、博客、实拍照片及提交压缩包另存于课程提交材料目录。
