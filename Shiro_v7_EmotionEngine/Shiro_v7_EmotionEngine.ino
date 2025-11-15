/*
 * =============================================================================
 * Shiro_v7_EmotionEngine.ino — (Modular State Machine)
 * [FIX v7.6] - Added Weather and Find Phone features.
 * - Added handleWeatherPolling() to the main loop.
 * =============================================================================
 */

// Core Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ChronosESP32.h>
#include <time.h>

// --- Shiro Project Files ---
#include "config.h"

// --- DEFINE Global Objects ---
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
ChronosESP32 chronos("Shiro_ESP32");

NotificationData g_Notification;
NavigationData g_Navigation;
StatusData g_Status;
// WeatherData g_Weather; // This is now created in config.h

// --- All other modules ---
#include "utils.h"
#include "animations.h"
#include "screens.h"
#include "touch.h"

// =====================================================================
//                           Setup
// =====================================================================
void setup() {
  Serial.begin(115200);
  delay(60);
  Serial.println("\n[Shiro_v7.6_EmotionEngine] Booting...");

  // Init hardware
  pinMode(PIN_TOUCH, INPUT);
  buzzerInit(); 

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("SSD1306 init failed.");
    while (true) { delay(500); }
  }
  display.clearDisplay();
  display.display();

  // --- Chronos init ---
  chronos.setConnectionCallback(onConnected);
  chronos.setNotificationCallback(onNotificationCb);
  chronos.setNotifyBattery(true);
  chronos.begin();

  drawIntroSplash(); 
  softChimeStartup(); 

  animation_Init(); 

  g_Status.lastInteraction = millis();
}

// =====================================================================
//                            Loop
// =====================================================================
void loop() {
  uint32_t now = millis(); 

  // 1. Service Chronos (required)
  chronos.loop();

  // 2. Poll hardware
  handleTouch(now); 

  // 3. Update global data
  g_Status.phoneBatPct = chronos.getPhoneBattery();
  g_Status.charging    = chronos.isPhoneCharging();
  
  // 4. Poll for navigation and weather
  handleNavigationPolling(now);
  handleWeatherPolling(now); // [NEW] Get weather updates

  // 5. ------ START DRAWING ------
  display.clearDisplay();

  // 6. Run the active screen's logic and drawing function
  handleScreen(now); 

  // 7. Push the final image to the screen
  display.display();
  // 8. ------ END DRAWING ------
}