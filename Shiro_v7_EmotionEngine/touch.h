#pragma once
#include "config.h"
#include "screens.h"
#include "animations.h"

// --- Internal Touch State ---
static bool     lastRawState    = false;
static bool     debouncedState  = false;
static uint32_t lastChangeMs    = 0;
static uint32_t pressStartMs    = 0;
static bool     isHolding       = false;
static uint8_t  tapCount        = 0;
static uint32_t lastTapTime     = 0;

// --- Global Touch Flags ---
bool g_SingleTap = false;
bool g_DoubleTap = false;
bool g_TripleTap = false; 
bool g_LongHold  = false;

// [NEW] Global flag for Find Phone
bool g_FindPhoneToggle = false;


void handleTouch(uint32_t now) {
  // 1. Reset flags
  g_SingleTap = false;
  g_DoubleTap = false;
  g_TripleTap = false;
  g_LongHold  = false;
  g_FindPhoneToggle = false; // [NEW] Reset this flag too

  // 2. Debounce the raw signal
  bool rawState = digitalRead(PIN_TOUCH);
  if (rawState != lastRawState) {
    lastChangeMs = now;
    lastRawState = rawState;
  }
  if (now - lastChangeMs < DEBOUNCE_MS) {
    return;
  }

  // 3. Process the stable (debounced) signal
  if (rawState != debouncedState) {
    debouncedState = rawState;
    if (debouncedState) {
      // --- PRESS EVENT ---
      pressStartMs = now;
      isHolding = true;
    } else {
      // --- RELEASE EVENT ---
      isHolding = false;
      uint32_t pressDuration = now - pressStartMs;

      if (pressDuration < LONG_HOLD_MS) {
        tapCount++;
        lastTapTime = now;
      }
    }
  }

  // 4. Handle continuous LONG HOLD
  if (isHolding && (now - pressStartMs >= LONG_HOLD_MS)) {
    g_LongHold = true;
    isHolding = false; 
    tapCount = 0;
  }

  // 5. Handle TAP events
  if (tapCount > 0 && (now - lastTapTime > MULTI_TAP_MS)) {
    if (tapCount == 1) {
      g_SingleTap = true;
    } else if (tapCount == 2) {
      g_DoubleTap = true;
    } else if (tapCount >= 3) { // Use >= 3 to catch 3 or more taps
      g_TripleTap = true;
    }
    tapCount = 0;
  }


  // 6. --- [NEW] Context-Aware Actions ---
  g_Status.lastInteraction = now; // Any touch is an interaction

  if (g_ActiveScreen == SCREEN_ANIM) {
    // --- We are on the ANIMATION screen ---
    if (g_SingleTap) {
      animation_WakeUp();
    }
    if (g_DoubleTap) {
      setScreen(SCREEN_TIME); // Go to the first utility screen
      buzzerTone(1200, 40);
      buzzerTone(1500, 40);
    }
    if (g_TripleTap) {
      animation_DoFeedInteraction(); // Feed Shiro
      buzzerTone(1100, 30);
      buzzerTone(1300, 30);
      buzzerTone(1500, 30);
    }
    if (g_LongHold) {
      animation_DoRubInteraction(); // "Rub" Shiro
    }
  } 
  else {
    // --- We are on a UTILITY screen (Time, Weather, etc.) ---
    if (g_SingleTap) {
      // On most utility screens, Single Tap means "Dismiss"
      if (g_ActiveScreen == SCREEN_FIND_PHONE) {
        g_FindPhoneToggle = true; // On this screen, it toggles the ringer
        buzzerTone(1800, 50);
      } else {
        setScreen(SCREEN_ANIM); // Dismiss
        buzzerTone(1000, 30);
      }
    }
    if (g_DoubleTap) {
      // Double Tap now cycles through utility screens
      if (g_ActiveScreen == SCREEN_TIME) {
        setScreen(SCREEN_WEATHER);
      } else if (g_ActiveScreen == SCREEN_WEATHER) {
        setScreen(SCREEN_FIND_PHONE);
      } else if (g_ActiveScreen == SCREEN_FIND_PHONE) {
        setScreen(SCREEN_TIME);
      } else {
        // From Notification or Nav, just go to Time
        setScreen(SCREEN_TIME);
      }
      buzzerTone(1200, 40);
    }
    // Long Hold and Triple Tap do nothing on utility screens
  }
}