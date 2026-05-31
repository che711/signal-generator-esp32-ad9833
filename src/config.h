#pragma once

// ── Wi-Fi ─────────────────────────────────────────────────
#define WIFI_SSID               "SkyNet"
#define WIFI_PASSWORD           "password"

// ── Web server ────────────────────────────────────────────
#define WEB_PORT                80
#define MDNS_HOSTNAME           "dds-gen"   // http://dds-gen.local

// ── WiFi watchdog ─────────────────────────────────────────
#define WIFI_RECONNECT_MS       10000
#define WIFI_CONNECT_TIMEOUT_MS 8000

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
#define DEBOUNCE_MS              5
#define LONG_PRESS_MS          800
#define ACCEL_THRESHOLD_MS      80    // быстрое вращение (мс между тиками)
#define ACCEL_MULTIPLIER        10

// ── Display ───────────────────────────────────────────────
#define DISPLAY_REFRESH_MS      33    // ~30 fps

// ── Autosave ──────────────────────────────────────────────
#define AUTOSAVE_MS           5000    // сохранить через 5 сек после изменения

// ── FreeRTOS tasks ────────────────────────────────────────
#define TASK_WEB_STACK        8192
#define TASK_WEB_PRIORITY        1
#define TASK_WEB_CORE            0    // WebUI на ядре 0
#define TASK_UI_CORE             1    // UI/encoder на ядре 1
