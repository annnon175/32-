# 实验 3：HAL + 按键中断控制流水灯

三只外接 LED 接 PA8、PA9、PA10，均属于 GPIOA，高电平点亮。PA0 使用内部上拉，按键可用杜邦线短接 PA0 与 GND 代替，下降沿触发 EXTI0。

程序特点：

- 中断函数只调用 HAL 的 EXTI 处理并切换 `g_running`；
- 使用 50 ms 时间间隔做简单按键去抖；
- 主循环使用 `HAL_GetTick()`，没有在中断中调用延时；
- 暂停时保持当前 LED，恢复后从当前步骤继续；
- 短接 PA0 到 GND 一下相当于按键按下；移开后再次短接可切换回运行状态。

在 PowerShell 中运行 `build.ps1` 可生成 ELF、HEX、BIN 和 MAP 文件。
