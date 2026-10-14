# 01_TIM2_Blink

TIM2更新中断每秒翻转PA0，亮1秒、灭1秒，完整周期2秒。

接线：PA0控制外接LED，LED短脚 → GND。板卡正常3.3V供电，BOOT0=0。

硬件下载打开同名.uvprojx，默认CMSIS-DAP；离线仿真打开_Simulator.uvprojx。

Core/Inc/lab_config.h集中放参数。PWM_FIXED_PERCENT=-1为呼吸，0、25、50、75、100为固定占空比。

GCC编译：PowerShell运行 .\build.ps1。Keil使用本机ARM Compiler 5.06。

IOC为配套配置。CubeMX重新生成请选择另一个目录，再移入业务代码；当前工程是在既有HAL工程上手工整理，不能只靠IOC自动生成呼吸逻辑。
