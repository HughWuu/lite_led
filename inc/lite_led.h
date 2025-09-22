/**
 * @file    lite_led.h
 * @brief   Lite LED Driver Framework
 *
 * This module provides a lightweight and flexible LED control framework,
 * designed for embedded systems with periodic polling.
 *
 * Features:
 *   - Support for multiple LEDs (defined in lite_led_cfg.h)
 *   - Configurable modes:
 *       * LED_MODE_OFF       : Always off
 *       * LED_MODE_ON        : Always on
 *       * LED_MODE_BLINK     : Periodic on/off
 *       * LED_MODE_BREATH    : Smooth breathing effect
 *       * LED_MODE_FADE_IN   : Gradual fade-in
 *       * LED_MODE_FADE_OUT  : Gradual fade-out
 *       * LED_MODE_ALTERNATE : Alternate between two LEDs
 *   - Duration control (auto stop after timeout)
 *   - Custom brightness callback for hardware abstraction
 * 
 * @author  HughWu
 * @date    2025-08-21
 * @version 1.0
 */

#ifndef __LITE_LED_H__
#define __LITE_LED_H__

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "lite_led_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LED_NUM              LED_MAX    // Number of LEDs
#define LED_MAX_BRIGHTNESS   (100)      //Max brightness percentage
#define LED_MIN_BRIGHTNESS   (0)        // Min brightness percentage
#define LED_BLOCK_FOREVER 0xFFFFFFFF    // Infinite block

// Number of LEDs
#define LED_ERROR_NONE              0
#define LED_ERROR_PARA_INVALID      -1
#define LED_ERROR_MODE_INVALID      -2
#define LED_ERROR_ALTERNATE_ID      -3

typedef void (*led_set_brt_f)(uint8_t percent);
typedef void (*led_dur_timeout_f)(void);

typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK,
    LED_MODE_BREATH,
    LED_MODE_FADE_IN,
    LED_MODE_FADE_OUT,
    LED_MODE_ALTERNATE,
} led_mode_e;

typedef enum {
    LED_STATE_OFF = 0,
    LED_STATE_ON,
} led_state_e;

typedef struct {
    led_mode_e mode;        /* LED mode: ON, OFF, BLINK, BREATH, FADE_IN, FADE_OUT, ALTERNATE */
    led_id_e alter_id;      /* LED ID to pair with in ALTERNATE mode */
    uint32_t on_ms;         /* ON duration in milliseconds (for BLINK) */
    uint32_t off_ms;        /* OFF duration in milliseconds (for BLINK) */
    uint32_t fade_ms;       /* Fade duration in milliseconds (for BREATH/FADE_IN/FADE_OUT) */
    uint32_t alternate_ms;  /* Alternate mode period in milliseconds */
    uint32_t duration_ms;   /* Total duration in milliseconds (0 = infinite) */
} led_cfg_t;

typedef struct {
    led_mode_e mode;
    led_id_e alter_id;
    size_t on_tick;
    size_t off_tick;
    size_t fade_tick;
    size_t alternate_tick;
    size_t duration_tick;
} led_inner_cfg_t;

typedef struct {
    uint8_t percent;    // Current brightness (%)
    led_state_e state;  // Current state
    size_t next_tick;   // Current state
    size_t remain_tick; // Remaining duration
    float phase;        // Current phase
    float phase_step;   // Step per tick
    bool dur_timeout;
} led_status_t;

typedef struct {
    led_id_e id;
    led_inner_cfg_t cfg;
    led_status_t stat;
    led_set_brt_f set_percent_cb;
    led_dur_timeout_f dur_timeout_cb;
} led_dev_t;

// ========== API ==========
int lite_led_init(uint8_t id, led_set_brt_f cb);
int lite_led_register_duration_timeout_cb(uint8_t id, led_dur_timeout_f cb);
int lite_led_write(uint8_t id, const led_cfg_t *cfg);
int lite_led_read(uint8_t id, led_status_t *status);
void lite_led_poll_handle(void);

#ifdef __cplusplus
}
#endif

#endif // __LITE_LED_H__
