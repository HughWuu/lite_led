/**
 * @file    lite_led_config.h
 * @brief   Lite LED driver configuration header file.
 *
 * This file defines board-specific LED configurations such as
 * polling period and LED ID mappings. Modify this file according
 * to your hardware platform.
 *
 * @note    Part of Lite LED driver framework.
 * @author  Hugh
 * @date    2025-08-23
 * @version 1.0
 */

#ifndef __LITE_LED_CONFIG_H__
#define __LITE_LED_CONFIG_H__

#include "lite_led.h"

#ifdef __cplusplus
extern "C" {
#endif

// LED polling period (ms)
#define LED_POLL_PERIOD_MS      (100)

// 1: use LUT for breath/fade, 0: use calculation
#define LED_BREATH_LUT_ENABLE   (1)

// LED ID list (update according to your hardware)
typedef enum {
    LED_GREEN = 0,
    LED_BLUE,
    LED_RED,
    LED_WHITE,

    LED_MAX,
    LED_INVALID,
} led_id_e;

#ifdef __cplusplus
}
#endif

#endif // __LITE_LED_CONFIG_H__
