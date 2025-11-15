#pragma once

/*
 * =============================================================================
 * animations.h - The Shiro v7 Emotion Engine
 * [FIX v7.5] - The "Virtual Pet" Update
 * 1. Adds "Over-Stimulation" logic to rubbing (DoRubInteraction).
 * 2. Adds "Hunger" timer and "Feeding" (DoFeedInteraction) via Triple-Tap.
 * 3. Uses cry.h for hunger and foody.h for feeding.
 * =============================================================================
 */

// 1. --- Define the AnimatedGIF Structure ---
#ifndef ANIMATED_GIF_DEFINED
#define ANIMATED_GIF_DEFINED
typedef struct {
    const uint8_t frame_count;
    const uint16_t width;
    const uint16_t height;
    const uint16_t* delays;         // Pointer to PROGMEM delays
    const uint8_t (* frames)[1024]; // Pointer to PROGMEM frames
} AnimatedGIF;
#endif // ANIMATED_GIF_DEFINED


// 2. --- Include ALL of your animation files ---
#include "cry.h"
#include "relaxed.h"
#include "angry.h"
#include "angry_2.h"
#include "hehe.h"
#include "confused.h"
#include "confused_2.h"
#include "happy.h"
#include "love.h"
#include "sleep.h"
#include "foody.h"
#include "frustrated.h"
#include "after_sleep.h"

// 3. --- Define Shiro's Emotions ---
enum Emotion {
  EMOTION_IDLE,       // Neutral, default
  EMOTION_HAPPY,
  EMOTION_ANGRY,
  EMOTION_SAD,        // [NEW] Used for hunger
  EMOTION_CONFUSED,
  EMOTION_SLEEPING
};

// 4. --- Define the Animation Player's State ---
enum PlayerState {
  STATE_STOPPED,
  STATE_PLAYING,
  STATE_INTERRUPT     // A high-priority clip (like LOVE)
};

// --- [NEW] Pet Interaction Timers & Counters ---
#define RUB_COOLDOWN_MS 10000     // 10 seconds to "forget" annoyance
#define RUBS_TO_ANNOY 4           // 4 rubs to get annoyed
#define RUBS_TO_ANGRY 6           // 6 rubs to get angry
#define HUNGER_TIMER_MS 3600000   // 1 hour to get hungry

// --- Global Animation/Emotion State ---
static Emotion g_CurrentEmotion = EMOTION_IDLE;
static PlayerState g_PlayerState = STATE_STOPPED;
static uint32_t g_AnimLastFrameTime = 0;
static int g_AnimCurrentFrame = 0;
static const AnimatedGIF* g_CurrentClip = nullptr;

static int g_RubCounter = 0;          // For over-stimulation
static uint32_t g_LastRubTime = 0;    // For rub cooldown
static uint32_t g_LastAteTime = 0;    // For hunger
static bool g_IsHungry = false;       // Hunger state


// =====================================================================
//                          Render Frame
// =====================================================================
void drawCurrentAnimationFrame() {
  if (g_CurrentClip == nullptr || g_PlayerState == STATE_STOPPED) {
    return;
  }
  
  const uint8_t* frame_ptr = g_CurrentClip->frames[g_AnimCurrentFrame];

  if (frame_ptr == nullptr) {
    g_PlayerState = STATE_STOPPED;
    return;
  }
  display.drawBitmap(0, 0, frame_ptr, g_CurrentClip->width, g_CurrentClip->height, WHITE);
}

// =====================================================================
//                        Animation Clip Control
// =====================================================================

void playClip(const AnimatedGIF* clip, PlayerState state) {
  if (clip == nullptr) {
    Serial.println("!!! ERROR: Tried to play a NULL animation clip!");
    g_PlayerState = STATE_STOPPED;
    return;
  }
  if (clip->frame_count == 0 || clip->delays == nullptr || clip->frames == nullptr) {
    Serial.println("!!! ERROR: Clip has 0 frames or null data!");
    g_PlayerState = STATE_STOPPED;
    return;
  }
  
  g_CurrentClip = clip;
  g_AnimCurrentFrame = 0;
  g_AnimLastFrameTime = millis();
  g_PlayerState = state;
}

// [NEW] This is the "rub" interaction with over-stimulation
void animation_DoRubInteraction() {
  uint32_t now = millis();

  // 1. Check if it's been a while. If so, forget the last rubs.
  if (now - g_LastRubTime > RUB_COOLDOWN_MS) {
    g_RubCounter = 0; // Reset counter
    Serial.println("[Emotion] Cooled down. Reset rub counter.");
  }

  // 2. Increment the rub counter
  g_RubCounter++;
  g_LastRubTime = now;
  Serial.print("[Emotion] Rub count: "); Serial.println(g_RubCounter);

  // 3. React based on how many times Shiro has been rubbed
  if (g_RubCounter < RUBS_TO_ANNOY) {
    // First 1-3 rubs are positive
    Serial.println("[Emotion] React: Love");
    playClip(&love_gif, STATE_INTERRUPT);
    g_CurrentEmotion = EMOTION_HAPPY;
    g_IsHungry = false; // Petting also counts as feeding
    g_LastAteTime = now;
  } 
  else if (g_RubCounter < RUBS_TO_ANGRY) {
    // Rubs 4-5 are annoying
    Serial.println("[Emotion] React: Frustrated");
    playClip(&frustrated_gif, STATE_INTERRUPT);
    g_CurrentEmotion = EMOTION_IDLE; // Not angry yet, but not happy
  }
  else {
    // 6+ rubs are too much!
    Serial.println("[Emotion] React: ANGRY!");
    playClip(&angry_gif, STATE_INTERRUPT);
    g_CurrentEmotion = EMOTION_ANGRY;
  }
}

// [NEW] This is the "feed" interaction (from triple-tap)
void animation_DoFeedInteraction() {
  Serial.println("[Emotion] React: Fed!");
  playClip(&foody_gif, STATE_INTERRUPT);
  g_IsHungry = false;
  g_LastAteTime = millis();
  g_CurrentEmotion = EMOTION_HAPPY;
  g_RubCounter = 0; // Feeding calms Shiro down
}

// Waking up resets annoyance and hunger
void animation_WakeUp() {
  g_RubCounter = 0; // Waking up resets annoyance
  
  if (g_CurrentEmotion == EMOTION_SLEEPING) {
     g_CurrentEmotion = EMOTION_IDLE;
     playClip(&after_sleep_gif, STATE_PLAYING);
  } else if (g_CurrentEmotion == EMOTION_CONFUSED) {
     g_CurrentEmotion = EMOTION_IDLE;
     playClip(&confused_2_gif, STATE_PLAYING);
  }
}

// Init the system
void animation_Init() {
  playClip(&happy_gif, STATE_PLAYING);
  g_CurrentEmotion = EMOTION_HAPPY;
  g_LastAteTime = millis(); // Just ate
}

// =====================================================================
//                       Main Animation State Machine
// =====================================================================
void handleAnimationState(uint32_t now) {

  // --- 1. Check for State Timers (Idle, Hunger) ---
  if (now - g_Status.lastInteraction > IDLE_TIMEOUT_MS) {
    if (g_CurrentEmotion != EMOTION_SLEEPING && g_CurrentEmotion != EMOTION_CONFUSED) {
      playClip(&sleep_gif, STATE_PLAYING);
      g_CurrentEmotion = EMOTION_SLEEPING;
    }
  }
  if (now - g_Status.lastInteraction > IDLE_SLEEP_MS) {
    if (g_CurrentEmotion == EMOTION_SLEEPING) {
       playClip(&confused_gif, STATE_PLAYING);
       g_CurrentEmotion = EMOTION_CONFUSED;
    }
  }
  
  // [NEW] Hunger Timer
  if (!g_IsHungry && (now - g_LastAteTime > HUNGER_TIMER_MS)) {
    Serial.println("[Emotion] Became hungry!");
    g_IsHungry = true;
    g_CurrentEmotion = EMOTION_SAD;
    playClip(&cry_gif, STATE_PLAYING); // Start crying
    g_PlayerState = STATE_STOPPED;     // Force a new clip next loop
  }
  

  // --- 2. Check if a clip is playing ---
  if (g_PlayerState == STATE_STOPPED) {
    // Nothing is playing. Let's pick a random idle animation
    
    // [NEW] Priority logic: Check for hunger first
    if (g_IsHungry) {
      playClip(&cry_gif, STATE_PLAYING);
      g_CurrentEmotion = EMOTION_SAD;
      return; // Stay on cry loop
    }
    
    // If not hungry, pick based on emotion
    int r = random(10);
    if (g_CurrentEmotion == EMOTION_HAPPY) {
      playClip(r < 5 ? &happy_gif : &hehe_gif, STATE_PLAYING);
    } else if (g_CurrentEmotion == EMOTION_ANGRY) {
      playClip(r < 5 ? &angry_gif : &frustrated_gif, STATE_PLAYING);
    } else if (g_CurrentEmotion == EMOTION_CONFUSED) {
      playClip(&confused_gif, STATE_PLAYING);
    } else if (g_CurrentEmotion == EMOTION_SLEEPING) {
      playClip(&sleep_gif, STATE_PLAYING);
    } else {
      // Neutral/Idle mood
      playClip(&relaxed_gif, STATE_PLAYING);
    }
    return;
  }


  // --- 3. A clip IS playing. Advance the frame. ---
  if (g_CurrentClip == nullptr) {
    g_PlayerState = STATE_STOPPED;
    return;
  }

  uint16_t frameDelay = pgm_read_word(&g_CurrentClip->delays[g_AnimCurrentFrame]);

  if (now - g_AnimLastFrameTime >= frameDelay) {
    g_AnimLastFrameTime = now;
    g_AnimCurrentFrame++;

    if (g_AnimCurrentFrame >= g_CurrentClip->frame_count) {
      g_AnimCurrentFrame = 0; // Loop animation
      
      if (g_PlayerState == STATE_INTERRUPT) {
        g_PlayerState = STATE_STOPPED;
      }
    }
  }
}