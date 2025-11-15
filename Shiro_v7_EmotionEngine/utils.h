#pragma once
#include "config.h"

// =====================================================================
//                          Buzzer (LEDC)
// =====================================================================
void buzzerInit() {
#if defined(ARDUINO_ARCH_ESP32)
  ledc_timer_config_t t = {
    .speed_mode = LEDC_LOW_SPEED_MODE,
    .duty_resolution = (ledc_timer_bit_t)BUZZ_TIMER_RES,
    .timer_num = LEDC_TIMER_0, .freq_hz = 2000, .clk_cfg = LEDC_AUTO_CLK
  };
  ledc_timer_config(&t);
  ledc_channel_config_t ch = {
    .gpio_num = PIN_BUZZ, .speed_mode = LEDC_LOW_SPEED_MODE,
    .channel = (ledc_channel_t)BUZZ_CHANNEL, .intr_type = LEDC_INTR_DISABLE,
    .timer_sel = LEDC_TIMER_0, .duty = 0, .hpoint = 0
  };
  ledc_channel_config(&ch);
#else
  pinMode(PIN_BUZZ, OUTPUT); digitalWrite(PIN_BUZZ, LOW);
#endif
}

void buzzerTone(uint16_t f, uint16_t ms) {
#if defined(ARDUINO_ARCH_ESP32)
  ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, f);
  ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)BUZZ_CHANNEL, BUZZ_SOFT_DUTY);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)BUZZ_CHANNEL);
  delay(ms); // Blocking, but ok for short sounds
  ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)BUZZ_CHANNEL, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)BUZZ_CHANNEL);
#else
  tone(PIN_BUZZ, f, ms); delay(ms); noTone(PIN_BUZZ);
#endif
}

void buzzerStop() {
#if defined(ARDUINO_ARCH_ESP32)
  ledc_set_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)BUZZ_CHANNEL, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, (ledc_channel_t)BUZZ_CHANNEL);
#else
  noTone(PIN_BUZZ);
#endif
}

void softChimeStartup() {
  buzzerTone(1047, 60); delay(25);
  buzzerTone(1319, 60); delay(25);
  buzzerTone(1568, 80); buzzerStop();
}

// Helper to get time string (HH:MM)
String getTimeString() {
  struct tm info;
  char tbuf[16] = "--:--";
  if (getLocalTime(&info)) {
    strftime(tbuf, sizeof(tbuf), "%H:%M", &info);
  }
  return String(tbuf);
}

// Helper to get date string (DD/MM)
String getDateString() {
  struct tm info;
  char dbuf[16] = "--/--";
  if (getLocalTime(&info)) {
    strftime(dbuf, sizeof(dbuf), "%d/%m", &info);
  }
  return String(dbuf);
}