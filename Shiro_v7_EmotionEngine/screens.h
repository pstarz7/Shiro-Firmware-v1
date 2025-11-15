#pragma once

#include "config.h"
#include "animations.h" 
#include "utils.h"
#include "bitmaps.h"    // We are still using the icons

extern bool g_FindPhoneToggle;

// =====================================================================
//                         Screen Manager
// =====================================================================

enum Screen {
  SCREEN_ANIM,
  SCREEN_TIME,
  SCREEN_NOTIFICATION,
  SCREEN_NAVIGATION,
  SCREEN_WEATHER,
  SCREEN_FIND_PHONE
};

Screen g_ActiveScreen = SCREEN_ANIM;

// Forward declarations
void drawScreen_Anim(uint32_t now);
void drawScreen_Time(uint32_t now);
void drawScreen_Notification(uint32_t now);
void drawScreen_Navigation(uint32_t now);
void drawScreen_Weather(uint32_t now);
void drawScreen_FindPhone(uint32_t now);

void setScreen(Screen newScreen) {
  g_ActiveScreen = newScreen;
  g_Status.lastInteraction = millis(); 
}

void handleScreen(uint32_t now) {
  switch (g_ActiveScreen) {
    case SCREEN_ANIM: drawScreen_Anim(now); break;
    case SCREEN_TIME: drawScreen_Time(now); break;
    case SCREEN_NOTIFICATION: drawScreen_Notification(now); break;
    case SCREEN_NAVIGATION: drawScreen_Navigation(now); break;
    case SCREEN_WEATHER: drawScreen_Weather(now); break;
    case SCREEN_FIND_PHONE: drawScreen_FindPhone(now); break;
  }
}

// =====================================================================
//                       Chronos Callbacks & Pollers
// =====================================================================

void onConnected(bool connected) {
  Serial.println(connected ? "[Chronos] Connected" : "[Chronos] Disconnected");
  if (connected) {
    softChimeStartup();
    g_Weather.city = chronos.getWeatherCity();
  } else {
    g_Weather.city = "Offline";
  }
}

void onNotificationCb(Notification n) {
  g_Notification.app    = n.app.length()     ? n.app : "App";
  g_Notification.sender = n.title.length()   ? n.title : "Sender";
  g_Notification.msg    = n.message.length() ? n.message : "Message here...";
  g_Notification.time   = getTimeString();
  
  setScreen(SCREEN_NOTIFICATION); 
  buzzerTone(1280, 70); delay(25); buzzerTone(1620, 80);
}

void handleNavigationPolling(uint32_t now) {
  static String lastDirText = "";
  Navigation nav = chronos.getNavigation();

  if (nav.active) {
    if (nav.directions != lastDirText) {
      g_Navigation.directions = nav.directions;
      g_Navigation.distance   = nav.distance.length() ? nav.distance : "---";
      g_Navigation.eta        = nav.eta.length() ? nav.eta : "--:--";
      g_Navigation.time       = getTimeString();
      lastDirText = nav.directions;

      setScreen(SCREEN_NAVIGATION);
      buzzerTone(980, 70); delay(25); buzzerTone(1180, 70);
    }
  }
  g_Navigation.active = nav.active;
}

void handleWeatherPolling(uint32_t now) {
  static uint32_t lastWeatherCheck = 0;
  if (now - lastWeatherCheck > 900000) { // 15 minutes
    lastWeatherCheck = now;
    
    if (chronos.isConnected() && chronos.getWeatherCount() > 0) {
      g_Weather.city = chronos.getWeatherCity();
      Weather w = chronos.getWeatherAt(0); 
      g_Weather.temp = String(w.temp);
      
      // [FIX] REMOVED THE BAD LINE: g_Weather.condition = w.main; 
      
      Serial.println("[Weather] Updated: " + g_Weather.city + ", " + g_Weather.temp + "C");
    }
  }
}


// =====================================================================
//                         Draw Functions
// =====================================================================

void drawIntroSplash() {
  display.clearDisplay();
  display.setTextSize(2); display.setTextColor(WHITE);
  display.setCursor(6, 18); display.print("Hey, I'm");
  display.setCursor(28, 40); display.print("Shiro");
  display.display();
  delay(1200);
}

void drawScreen_Anim(uint32_t now) {
  handleAnimationState(now);
  drawCurrentAnimationFrame();
}

// Helper function for blinking colon
void drawBlinkingColon(uint32_t now) {
  static bool showColon = true;
  static uint32_t lastBlink = 0;
  
  if (now - lastBlink > 500) {
    showColon = !showColon;
    lastBlink = now;
  }
  
  if (showColon) {
    display.drawRect(62, 16, 4, 4, WHITE);
    display.drawRect(62, 26, 4, 4, WHITE);
  }
}

// Professional Time Screen
void drawScreen_Time(uint32_t now) {
  struct tm info;
  char hbuf[4];
  char mbuf[4];

  if (getLocalTime(&info)) { 
    strftime(hbuf, sizeof(hbuf), "%H", &info);
    strftime(mbuf, sizeof(mbuf), "%M", &info);
  } else { 
    strcpy(hbuf, "--");
    strcpy(mbuf, "--");
  }
  
  // Draw Time
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(16, 14);
  display.print(hbuf);
  display.setCursor(76, 14);
  display.print(mbuf);

  // Blinking Colon
  drawBlinkingColon(now);

  // --- Bottom Bar ---
  display.drawFastHLine(0, 44, 128, WHITE);
  
  // Date
  display.drawBitmap(8, 49, icon_calendar_8x8, 8, 8, WHITE);
  display.setTextSize(1);
  display.setCursor(22, 50);
  display.print(getDateString());

  // Battery
  int pct = g_Status.phoneBatPct;
  int bx = 78; int by = 49; int bw = 30; int bh = 10;
  display.drawRoundRect(bx, by, bw, bh, 2, WHITE);
  display.drawRect(bx + bw, by + 2, 2, bh - 4, WHITE); 

  if (pct >= 0) {
    int fill = map(pct, 0, 100, 0, bw - 4);
    display.fillRect(bx + 2, by + 2, fill, bh - 4, WHITE);
  }
  
  if (g_Status.charging && pct < 100) { 
     display.drawBitmap(bx + 10, by + 1, icon_bolt_8x8, 8, 8, BLACK);
  }
}

void drawScreen_Notification(uint32_t now) {
  // Top Bar
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4, 3);
  display.print(g_Notification.sender); 
  display.setCursor(98, 3);
  display.print(g_Notification.time);   
  display.drawFastHLine(0, 12, 128, WHITE);

  // Main rounded rectangle
  display.drawRoundRect(0, 14, 128, 50, 7, WHITE);
  
  // Message text
  String msg = g_Notification.msg;
  String line1 = msg;
  String line2 = "";
  int maxChars = 20;

  if (msg.length() > maxChars) {
      int break_pos = -1;
      for (int i = maxChars; i > 0; i--) {
        if (msg.charAt(i) == ' ') { break_pos = i; break; }
      }
      if (break_pos != -1) {
        line1 = msg.substring(0, break_pos);
        line2 = msg.substring(break_pos + 1);
      } else {
        line1 = msg.substring(0, maxChars);
        line2 = msg.substring(maxChars);
      }
  }
  if (line2.length() > maxChars) {
    line2 = line2.substring(0, maxChars - 3) + "...";
  }
  
  display.setCursor(8, 20); display.print(line1);
  display.setCursor(8, 30); display.print(line2);

  // App name
  int appNameX = max(4, (int)(124 - (g_Notification.app.length() * 6)));
  display.setCursor(appNameX, 52);
  display.print(g_Notification.app);
}

// Helper function to draw the correct nav arrow
void drawNavArrow(String direction) {
  String dir = direction;
  dir.toLowerCase();

  if (dir.indexOf("left") > -1) {
    display.drawBitmap(108, 2, icon_arrow_left_16x16, 16, 16, WHITE);
  } else if (dir.indexOf("right") > -1) {
    display.drawBitmap(108, 2, icon_arrow_right_16x16, 16, 16, WHITE);
  } else if (dir.indexOf("destination") > -1) {
    display.drawBitmap(108, 2, icon_destination_16x16, 16, 16, WHITE);
  } else {
    // Default for "straight", "head", "continue"
    display.drawBitmap(108, 2, icon_arrow_up_16x16, 16, 16, WHITE);
  }
}

// Professional Navigation Screen
void drawScreen_Navigation(uint32_t now) {
  // Top bar
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4, 3);
  display.print("Navigation");

  drawNavArrow(g_Navigation.directions);

  // Main text
  display.setTextSize(2);
  display.setCursor(4, 20);
  String dir = g_Navigation.directions;
  if (dir.length() > 10) { dir = dir.substring(0, 10); }
  display.print(dir);
  
  // Bottom Box
  display.drawRoundRect(0, 38, 128, 26, 7, WHITE);
  
  display.setTextSize(1);
  display.setCursor(4, 44); display.print("Dist: " + g_Navigation.distance);
  display.setCursor(4, 54); display.print("Time: " + g_Navigation.time);
  display.setCursor(68, 54); display.print("ETA: " + g_Navigation.eta);
}


// [FIX] Professional Weather Screen (without condition text)
void drawScreen_Weather(uint32_t now) {
  // Top Bar (City Name)
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.drawRoundRect(0, 0, 128, 18, 5, WHITE);
  display.setCursor(6, 6);
  display.print(g_Weather.city);
  
  display.setCursor(84, 6);
  display.print("Weather");
  
  // Main Box
  display.drawRoundRect(0, 22, 128, 42, 7, WHITE);

  // [FIX] We don't know the condition, so we don't draw an icon.
  // Instead, we center the temperature in the box.
  display.setTextSize(3);
  display.setCursor(18, 34); 
  display.print(g_Weather.temp + "'C");

  // [FIX] All lines related to drawing g_Weather.condition are removed.
}

// Professional Find Phone Screen
void drawScreen_FindPhone(uint32_t now) {
  static bool isFinding = false;
  
  if (g_FindPhoneToggle) {
    isFinding = !isFinding; 
    chronos.findPhone(isFinding); 
    Serial.print("[Find Phone] Toggled to: "); Serial.println(isFinding);
  }

  // Draw the UI
  display.drawBitmap(20, 24, icon_bell_16x16, 16, 16, WHITE);
  
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  if (isFinding) {
    display.setCursor(48, 16);
    display.print("Ringing!");
    display.setTextSize(1);
    display.setCursor(48, 40);
    display.print("Tap to stop");
  } else {
    display.setCursor(48, 16);
    display.print("Find");
    display.setCursor(48, 30);
    display.print("Phone?");
  }
}