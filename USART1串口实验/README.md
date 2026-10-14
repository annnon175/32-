# STM32F103C8T6 USART1串口实验

三个独立HAL工程，默认115200、8N1，连续发送`hello windows!`加CRLF，接收`#`暂停、`*`恢复。

| 方式 | Keil工程 | 暂停位置 |
|---|---|---|
| 轮询 | [01_HAL_Polling.uvprojx](01_HAL_Polling/01_HAL_Polling.uvprojx) | 当前字节后 |
| 中断 | [02_HAL_Interrupt.uvprojx](02_HAL_Interrupt/02_HAL_Interrupt.uvprojx) | 当前16字节行后 |
| DMA发送+中断接收 | [03_HAL_DMA.uvprojx](03_HAL_DMA/03_HAL_DMA.uvprojx) | 当前256字节块后 |

## 接线

- PA9（TX）接串口模块RX；PA10（RX）接模块TX；GND共地。
- 实测使用Horco DAP自带串口，Windows识别为COM4。其他电脑按实际COM端口选择。
- SWD下载使用PA13、PA14和GND，供电按核心板与DAP实际接法连接。
- 旧流水灯实验PA9/PA10上的LED线先拆掉；MCU只接兼容3.3V IO的TTL/CMOS侧。

## 打开和编译

各工程包含HAL/CMSIS依赖、启动文件、链接脚本、Keil `.uvprojx`及调试选项`.uvoptx`。
Keil选择Arm Compiler5、CMSIS-DAP下载；备用GNU Arm构建在各目录执行：

```powershell
.uild.ps1
```

内部HSI 8MHz / 2 × 16 = 64MHz，APB1=32MHz，APB2=64MHz。无需依赖板载外部晶振。
`Core/Inc/lab_config.h`设置波特率与发送间隔，默认`UART_BAUD=115200U`、`TX_DELAY_MS=0U`。
将间隔改为50U可以做对照；DMA可改460800U，电脑端需同步改波特率。

配套`.ioc`已用CubeMX 6.18.1加载并保存。程序是在旧HAL工程基础上修改，重新生成配置时另选目录，业务代码按USER CODE区域迁入。
中断目录内的Simulator配置副本供软件仿真尝试；USART位波形和光标测速尚未完成，不算已通过的验证项。

## 已完成测试（2026-10-13）

三个默认工程均通过Keil和GNU Arm编译，Keil为0错误、0警告。
用真实C8T6和Horco DAP烧录后，固件Flash回读与BIN一致，以下四组采样格式检查、暂停与恢复均通过。

| 模式/波特率 | 采样秒数 | 收到字节 | 载荷Byte/s |
|---|---:|---:|---:|
| 轮询115200 | 2.0021 | 20929 | 10453.3 |
| 中断115200 | 2.0031 | 22913 | 11438.5 |
| DMA115200 | 2.0028 | 23041 | 11504.5 |
| DMA460800 | 2.0002 | 92097 | 46042.8 |

这张表是Windows 11电脑端的载荷采样速度，不是波形测出的波特率。重复字符串格式正确不能单独排除整行丢失。
双电脑RS-232文件传输、不接GND对照和Keil位波形读数尚未完成。

HAL/CMSIS许可保留在各工程Drivers目录。完整报告、照片与实测日志保存在本地作业材料中。
