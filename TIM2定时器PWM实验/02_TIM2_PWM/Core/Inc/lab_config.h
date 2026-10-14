#ifndef LAB_CONFIG_H
#define LAB_CONFIG_H

/* HSI 8MHz / 2 * 16 = 64MHz；APB1分频2，TIM2仍为64MHz。 */
#define BLINK_PSC          63999U
#define BLINK_ARR          999U       /* 每1秒翻转，亮灭完整周期2秒 */
#define PWM_PSC            63U
#define PWM_COUNTS         1000U     /* 64MHz / 64 / 1000 = 1kHz */
#define BREATH_STEPS       200U
#define BREATH_STEP_MS     4U        /* 200 * 4ms * 2 = 1600ms */
#define PWM_FIXED_PERCENT  (-1)      /* -1呼吸；0~100固定占空比 */

#if PWM_FIXED_PERCENT < -1 || PWM_FIXED_PERCENT > 100
#error PWM_FIXED_PERCENT must be -1 or 0..100
#endif
#endif
