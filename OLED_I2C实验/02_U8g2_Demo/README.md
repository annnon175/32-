# 02_U8g2_Demo

打开同名uvprojx编译，使用CMSIS-DAP下载。GCC：PowerShell运行 .\build.ps1。
硬件I2C1：PB6/SCL、PB7/SDA；OLED 0x3C，AHT20 0x38。USART1 PA9/PA10，115200 8N1。
Core/Inc/lab_config.h为姓名、学号、长句和地址配置；Core/Src/main.c为演示主循环；demo.c为各页绘图。
源码为UTF-8，中文显示由实验小字库完成；增加汉字需重新生成字体。
