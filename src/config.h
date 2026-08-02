#pragma once

// ── Wi-Fi ─────────────────────────────────────────────────
#define WIFI_SSID      "SkyNet"
#define WIFI_PASSWORD  "password"

// ── Web server ────────────────────────────────────────────
#define WEB_PORT                80
#define MDNS_HOSTNAME           "dds-gen"   // http://dds-gen.local

// ── WiFi watchdog ─────────────────────────────────────────
#define WIFI_RECONNECT_MS       10000
#define WIFI_CONNECT_TIMEOUT_MS 8000

// ── FreeRTOS tasks ────────────────────────────────────────
#define TASK_WEB_STACK        8192
#define TASK_WEB_PRIORITY        1
#define TASK_WEB_CORE            0    // WebUI runs on core 0

// ── Sweep ─────────────────────────────────────────────────
#define SWEEP_TICK_MS      10    // frequency recalc period (100 steps/s)
#define SWEEP_MIN_MS      200    // minimum sweep duration
#define SWEEP_MAX_MS  3600000    // maximum 1 hour

// ── Autosave ──────────────────────────────────────────────
#define AUTOSAVE_MS           5000    // save 5 s after the last change

// ── NVS namespace ─────────────────────────────────────────
#define NVS_NAMESPACE           "ddsgen"

// ── SPI (AD9833) ──────────────────────────────────────────
#define GEN_SCK_PIN             18
#define GEN_MOSI_PIN            23
#define GEN_CS_PIN               5

// ── I2C (SSD1306) ─────────────────────────────────────────
#define OLED_SDA_PIN            21
#define OLED_SCL_PIN            22
#define OLED_ADDR               0x3C

// ── KY-040 encoder ────────────────────────────────────────
#define ENC_CLK_PIN             34
#define ENC_DT_PIN              35
#define ENC_SW_PIN              32

// ── Encoder timing ────────────────────────────────────────
#define BTN_DEBOUNCE_MS   20    // ms of stable level (KY-040 bounces 5-30 ms)
#define LONG_PRESS_MS    800
#define ACCEL_THRESHOLD   80    // ms between detents to count as fast rotation
#define ACCEL_MULTIPLIER  10    // step x 10 while spinning fast
