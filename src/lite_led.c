/**
 * @file    lite_led.c
 * @brief   Lite LED Driver Implementation
 *
 * This file implements a lightweight LED driver framework, supporting
 * multiple effects such as ON/OFF, BLINK, BREATH, FADE, and ALTERNATE.
 *
 * Core Mechanism:
 *   - Each LED is managed by a `led_dev_t` structure.
 *   - LED states are updated periodically by `lite_led_poll_handle()`.
 *   - Hardware-specific brightness control is abstracted via callback.
 *
 * Key Features:
 *   - Multiple LED instances (`g_led_list`) managed statically.
 *   - Tick-based timing (based on LED_POLL_PERIOD_MS).
 *   - Cosine-based brightness calculation for smooth breathing/fading.
 *   - Duration management (auto-stop after given time).
 *
 * @author  HughWu
 * @date    2025-08-23
 * @version 1.0
 *
 * @note
 *   - Requires lite_button_cfg.h for configuration.
 *   - Only lite_button.h should be included by application code.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "lite_led.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#define LED_PI        M_PI         // π
#define LED_2PI       (2.0 * M_PI) // 2π

static led_dev_t g_led_list[LED_NUM] = {0};

#if LED_BREATH_LUT_ENABLE
#define LED_TABLE_SIZE 128
static const uint8_t g_led_sin_table[LED_TABLE_SIZE + 1] = {
    0,  1,  2,  3,  4,  5,  7,  8, 10, 11, 13, 15, 16, 18, 20, 22,
   24, 26, 28, 30, 32, 34, 37, 39, 41, 44, 46, 49, 51, 54, 56, 59,
   61, 64, 67, 69, 72, 75, 77, 80, 83, 86, 88, 91, 94, 97,100,100,
  100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
  100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
   97, 94, 91, 88, 86, 83, 80, 77, 75, 72, 69, 67, 64, 61, 59, 56,
   54, 51, 49, 46, 44, 41, 39, 37, 34, 32, 30, 28, 26, 24, 22, 20,
   18, 16, 15, 13, 11, 10,  8,  7,  5,  4,  3,  2,  1,  0,  0,  0
};

static uint8_t lite_led_get_percent_from_phase(float phase)
{
    float step = LED_2PI / LED_TABLE_SIZE;
    size_t index = (size_t)(phase / step);
    if (index > LED_TABLE_SIZE) index = LED_TABLE_SIZE;

    return g_led_sin_table[index];
}
#endif

/**
 * @brief Initialize an LED instance
 * 
 * @param id LED ID (0 ~ LED_NUM-1)
 * @param cb Callback for brightness setting (0-100%)
 * @return int Error code (0: success, <0: failure)
 */
int lite_led_init(uint8_t id, led_set_brt_f cb)
{
    if (id >= LED_NUM || cb == NULL) return -1;

    memset(&g_led_list[id], 0, sizeof(led_dev_t));
    g_led_list[id].id = id;
    g_led_list[id].set_percent_cb = cb;

    return LED_ERROR_NONE;
}

/**
 * @brief Configure LED behavior
 *
 * @param id LED ID
 * @param cfg LED configuration
 * @return int Error code
 */
int lite_led_write(uint8_t id, const led_cfg_t *cfg)
{
    led_dev_t *led = NULL;

    if (id >= LED_MAX || cfg == NULL) return LED_ERROR_PARA_INVALID;

    led = &g_led_list[id];

    led->cfg.mode = cfg->mode;
    led->cfg.alter_id = cfg->alter_id;
    led->cfg.on_tick = cfg->on_ms / LED_POLL_PERIOD_MS;
    led->cfg.off_tick = cfg->off_ms / LED_POLL_PERIOD_MS;
    led->cfg.fade_tick = cfg->fade_ms / LED_POLL_PERIOD_MS;
    led->cfg.alternate_tick = cfg->alternate_ms / LED_POLL_PERIOD_MS;
    led->cfg.duration_tick = cfg->duration_ms / LED_POLL_PERIOD_MS;

    memset(&(led->stat), 0, sizeof(led->stat));
    led->stat.remain_tick = led->cfg.duration_tick;

    switch (cfg->mode) {
        case LED_MODE_ON:
            break;
        case LED_MODE_OFF:
            break;
        case LED_MODE_BLINK:
            break;
        case LED_MODE_BREATH:
        case LED_MODE_FADE_IN:
        case LED_MODE_FADE_OUT:
            // Phase step controls brightness update speed
            led->stat.phase_step = (float)LED_PI * LED_POLL_PERIOD_MS / (float)cfg->fade_ms;
            if (led->stat.phase_step <= 0.0f) {
                led->stat.phase_step = (float)LED_PI / (float)LED_MAX_BRIGHTNESS;
            }
            if (cfg->mode == LED_MODE_FADE_OUT) {
                led->stat.percent = LED_MAX_BRIGHTNESS;
                led->stat.phase = LED_PI;
                led->stat.phase_step = -led->stat.phase_step;
            } else {
                led->stat.percent = LED_MIN_BRIGHTNESS;
                led->stat.phase = 0.0f;
            }
            break;
        case LED_MODE_ALTERNATE:
            if (id == cfg->alter_id) return LED_ERROR_ALTERNATE_ID;
            break;
        default:
            return LED_ERROR_MODE_INVALID;
    }

    return LED_ERROR_NONE;
}

/**
 * @brief Read LED current status
 *
 * @param id LED ID
 * @param status Output status
 * @return int Error code
 */
int lite_led_read(uint8_t id, led_status_t *status)
{
    if (id >= LED_NUM || status == NULL) return LED_ERROR_PARA_INVALID;

    *status = g_led_list[id].stat;

    return LED_ERROR_NONE;
}

/**
 * @brief Periodic LED state update
 *
 * This function should be called every LED_POLL_PERIOD_MS.
 * It updates LED states, handles timers, and triggers brightness callbacks.
 */
void lite_led_poll_handle(void)
{
    led_dev_t *led = NULL;

    for (size_t i = 0; i < LED_NUM; i++) {
        led = &g_led_list[i];
        if (led->set_percent_cb == NULL) continue;

        // Duration handling
        if (led->stat.remain_tick != 0) {
            led->stat.remain_tick--;
            if (led->stat.remain_tick == 0) {
                led->cfg.mode = LED_MODE_OFF;
                led->stat.next_tick = 0;
                continue;
            }
        }

        // Tick countdown
        if (led->stat.next_tick == LED_BLOCK_FOREVER) continue;
        if (led->stat.next_tick != 0) {
            led->stat.next_tick--;
            if (led->stat.next_tick != 0) continue;
        }

        // Mode handling
        switch (led->cfg.mode) {
            case LED_MODE_OFF:
                led->stat.state = LED_STATE_OFF;
                led->stat.percent = LED_MIN_BRIGHTNESS;
                led->stat.next_tick = LED_BLOCK_FOREVER;
                break;
            case LED_MODE_ON:
                led->stat.state = LED_STATE_ON;
                led->stat.percent = LED_MAX_BRIGHTNESS;
                led->stat.next_tick = LED_BLOCK_FOREVER;
                break;
            case LED_MODE_BLINK:
                if (led->stat.state == LED_STATE_OFF) {
                    led->stat.next_tick = led->cfg.on_tick;
                    led->stat.percent = LED_MAX_BRIGHTNESS;
                } else {
                    led->stat.next_tick = led->cfg.off_tick;
                    led->stat.percent = LED_MIN_BRIGHTNESS;
                }
                led->stat.state = !(led->stat.state);
                break;
            case LED_MODE_FADE_IN:
            case LED_MODE_FADE_OUT:
            case LED_MODE_BREATH:
                led->stat.phase += led->stat.phase_step; // Phase update
                if (led->cfg.mode == LED_MODE_BREATH) {
                    if (led->stat.phase >= LED_2PI) led->stat.phase -= LED_2PI;
                } else if (led->cfg.mode == LED_MODE_FADE_IN) {
                    if (led->stat.phase >= LED_PI) {
                        led->stat.phase = LED_PI;
                        led->stat.next_tick = LED_BLOCK_FOREVER;
                    }
                } else if (led->cfg.mode == LED_MODE_FADE_OUT) {
                    if (led->stat.phase <= 0.0f) {
                        led->stat.phase = 0.0f;
                        led->stat.next_tick = LED_BLOCK_FOREVER;
                    }
                }
                // Brightness update (cosine wave)
#if LED_BREATH_LUT_ENABLE
                led->stat.percent = lite_led_get_percent_from_phase(led->stat.phase);
#else
                led->stat.percent = (uint8_t)((1 - cos(led->stat.phase)) / 2.0 * LED_MAX_BRIGHTNESS);
#endif
                if (led->stat.percent >= LED_MAX_BRIGHTNESS) led->stat.percent = LED_MAX_BRIGHTNESS;
                break;
            case LED_MODE_ALTERNATE:
                if (led->cfg.alter_id >= LED_NUM) break;
                led->stat.next_tick = led->cfg.alternate_tick;
                if (led->id < led->cfg.alter_id) {
                    led->stat.state = !(led->stat.state);
                    led->stat.percent = (led->stat.state == LED_STATE_ON) ? LED_MAX_BRIGHTNESS : LED_MIN_BRIGHTNESS;
                } else {
                    led->stat.state = !(g_led_list[led->cfg.alter_id].stat.state);
                    led->stat.percent = (led->stat.state == LED_STATE_ON) ? LED_MAX_BRIGHTNESS : LED_MIN_BRIGHTNESS;
                }
                break;
            default:
                break;
        }

        // Update brightness
        led->set_percent_cb(led->stat.percent);
    }
}
