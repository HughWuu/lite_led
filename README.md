# lite_led

轻量级 LED 驱动框架，支持多种 LED 效果，包括常亮、闪烁、呼吸、渐亮/渐灭和交替闪烁，以及可设置LED效果的维持时间。  

---

## 特性

- 多 LED 实例管理（静态数组 `g_led_list`）
- 基于 tick 的时间控制 (`LED_POLL_PERIOD_MS`)
- Cosine 曲线实现平滑呼吸和渐变效果
- 持续时间控制，可自动停止 LED
- 可通过回调函数驱动硬件亮度（0~100%）

可配置参数如下：
typedef struct {
    led_mode_e mode;        /* LED 模式: 常亮/闪烁/呼吸/渐亮/渐灭/交替 */
    led_id_e alter_id;      /* 交替模式时对应的另一个 LED ID */
    uint32_t on_ms;         /* 闪烁模式下点亮时间 (ms) */
    uint32_t off_ms;        /* 闪烁模式下熄灭时间 (ms) */
    uint32_t fade_ms;       /* 呼吸/渐亮/渐灭模式下渐变时间 (ms) */
    uint32_t alternate_ms;  /* 交替模式周期时间 (ms) */
    uint32_t duration_ms;   /* 模式效果持续时间 (ms)，0 表示无限 */
} led_cfg_t

---

## 文件结构

lite_led/
├── lite_led.h // 驱动头文件
├── lite_led_cfg.h // LED 配置头文件
├── lite_led.c // 驱动实现
└── README.md

## 使用示例

详见example.c文件