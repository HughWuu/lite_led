#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "lite_led.h"
#include "lite_led_cfg.h"

bool g_100ms_flag = false;

// ====== 假设引入你的 led 驱动头文件 ======
#include "lite_led.h"

// 模拟硬件回调：实际项目中这里应该是 PWM 设置亮度
static void set_led0_percent(uint8_t percent)
{
    printf("LED0 brightness = %u%%\n", percent);
}

static void set_led1_percent(uint8_t percent)
{
    printf("LED1 brightness = %u%%\n", percent);
}

static void set_led2_percent(uint8_t percent)
{
    printf("LED2 brightness = %u%%\n", percent);
}

static void set_led3_percent(uint8_t percent)
{
    printf("LED3 brightness = %u%%\n", percent);
}

void led_blue_dur_timeout_callback(void)
{

}

int main(void)
{
    // 0. LED hardware init

    // 1. 初始化 LED
    lite_led_init(LED_GREEN, set_led0_percent);
    lite_led_init(LED_BLUE, set_led1_percent);
    lite_led_init(LED_RED, set_led2_percent);
    lite_led_init(LED_WHITE, set_led3_percent);

    // 2. 配置 LED0 = 呼吸灯 (2s 一个呼吸周期，永久运行)
    led_cfg_t breath_cfg = {
        .mode         = LED_MODE_BREATH,
        .fade_ms      = 1000,      // 0%-100%的时间fade_ms
        .duration_ms  = 0,         // 0 表示永久运行
    };
    lite_led_write(LED_GREEN, &breath_cfg);

    // 3. 配置 LED1 = 闪烁灯 (亮200ms，灭800ms，运行5秒后自动关)
    led_cfg_t blink_cfg = {
        .mode         = LED_MODE_BLINK,
        .on_ms        = 200,
        .off_ms       = 800,
        .duration_ms  = 5000,     // 5 秒后关
    };
    lite_led_write(LED_BLUE, &blink_cfg);
    // .duration_ms时间结束后回调led_blue_dur_timeout_callback()函数
    lite_led_register_duration_timeout_cb(LED_BLUE, led_blue_dur_timeout_callback);

    // 5. 再设置交替闪烁模式：LED0 和 LED1 交替亮灭
    led_cfg_t alternate_cfg = {
        .mode          = LED_MODE_ALTERNATE,
        .alternate_ms  = 500,     // 500ms 切换一次
        .duration_ms   = 3000,    // 3秒后结束
    };
    alternate_cfg.alter_id = LED_WHITE; //交替LED ID
    lite_led_write(LED_RED, &alternate_cfg);
    alternate_cfg.alter_id = LED_RED;
    lite_led_write(LED_WHITE, &alternate_cfg);

    /* 模拟主循环 */
    while(1) {
        if (g_100ms_flag) { // 每 100ms 调用一次
            lite_led_poll(); // LED_POLL_PERIOD_MS配置需要与轮询周期一致
        }
    }

    return 0;
}
