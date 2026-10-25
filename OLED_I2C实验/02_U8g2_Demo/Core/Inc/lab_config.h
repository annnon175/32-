#ifndef LAB_CONFIG_H
#define LAB_CONFIG_H

/* 上课演示主要改这里。地址按7位写，HAL调用时再左移1位。 */
#define STUDENT_NAME       "敖文浩"
#define STUDENT_ID         "632407030510"
#define SCROLL_TEXT        "Hello，欢迎来到重庆交通大学物联网205实训室！"
#define OLED_ADDR          0x3CU
#define AHT20_ADDR         0x38U
#define ENABLE_AHT20       0U  /* 现在没有传感器，不访问0x38，也不自动显示温湿度页。 */
#define I2C_SPEED_HZ       100000U
#define I2C_TIMEOUT_MS     30U
#define FRAME_INTERVAL_MS  40U
#define PAGE_INTERVAL_MS   8000U
#define SENSOR_INTERVAL_MS 2000U
#define START_PAGE         0U  /* 自动轮播，U8g2先显示头像。1~7可固定演示某页。 */

#endif
