#pragma once

// ---------------- Pins ----------------
static const int PIN_SDA   = 21;
static const int PIN_SCL   = 22;
static const int PIN_TOUCH = 13;
static const int PIN_BUZZ  = 25;

// ---------------- OLED ----------------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define OLED_I2C_ADDR 0x3C

// ---------------- Buzzer (LEDC) ----------------
#if defined(ARDUINO_ARCH_ESP32)
  #include "driver/ledc.h"
  static const uint8_t  BUZZ_CHANNEL   = 0;
  static const uint8_t  BUZZ_TIMER_RES = 8;
  static const uint16_t BUZZ_SOFT_DUTY = 60;
#endif

// ---------------- Touch Timings ----------------
static const uint16_t DEBOUNCE_MS     = 35;
static const uint16_t LONG_HOLD_MS    = 1500;
static const uint16_t MULTI_TAP_MS    = 350;

// ---------------- App Timings ----------------
static const uint32_t IDLE_TIMEOUT_MS  = 45000;
static const uint32_t IDLE_SLEEP_MS    = 120000;

// ---------------- Data Structs (Blueprints) ----------------
struct NotificationData {
  String app, sender, msg, time;
};

struct NavigationData {
  String directions = "";
  String distance   = "";
  String eta        = "";
  String time       = "";
  String then_dir   = "";
  bool   active     = false;
};

struct StatusData {
  int  phoneBatPct = -1;
  bool charging    = false;
  uint32_t lastInteraction = 0;
};

// [NEW] Struct to hold weather data
struct WeatherData {
  String city = "Loading...";
  String temp = "--";
  String condition = "Clear";
} g_Weather;