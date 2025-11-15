## Shiro Firmware version-1  

# Shiro - Your ESP32 Desk Pet 🤖

Hello! This is the firmware (the code) for Shiro, a little robot friend that lives on your desk.

I built this project to learn about electronics and coding. I had some basic knowledge but got a *lot* of help from AI to get started. After that, it was a ton of debugging and what I call "vibe coding"—just trying things out until all the errors finally went away.

This is my tiny step to making a real robot pet, and I'm sharing the code so you can build one, too!

![App Screenshot](https://github.com/pstarz7/Shiro-Firmware-v1/blob/main/last-img.png?raw=true)


## What Can Shiro Do? (Features)

Shiro is more than just a cute face. It's a "smart" pet that connects to your phone!
* **Has Emotions:** Shiro has an "Emotion Engine." It gets happy, sleepy, confused, and even hungry (it will cry!).
* **Reacts to Your Touch:** You can "pet" Shiro with a long press, but don't rub too much or it will get annoyed and angry!
* **Needs to be Fed:** After an hour, Shiro gets hungry. You have to "feed" it by **triple-tapping** the sensor to play the `foody.h` animation.
* **Shows Phone Notifications:** Connects to your phone with Bluetooth and shows your messages from apps like WhatsApp or Instagram.
* **Shows Time, Date & Battery:** You can double-tap to see a professional-looking clock and your phone's battery level.
* **Shows the Weather:** Shows the current temperature for your city.
* **Shows Map Directions:** When you use Google Maps, Shiro will show the next turn and an arrow on its screen.
* **Finds Your Phone:** Has a screen that will make your phone ring when you lose it.

---

## 1. "Shopping List" (Components)

To build Shiro, you only need a few simple parts.
* **The Brain:** 1 x **ESP32** (I use an ESP32 WROOM 32 Dev Module. This is the main computer chip.)
* **The Face:** 1 x **OLED Display 128x64 (SSD1306)** (Make sure it has 4 pins: VCC, GND, SCL, SDA).
* **The Sensor:** 1 x **TTP223 Touch Sensor** (This is the 'petting' sensor. It usually has 3 pins: VCC, GND, SIG).
* **The Voice:** 1 x **Active Buzzer** (This makes the beep sounds. It has a `+` and `-` pin).
* **The "Body":** A **Breadboard** and some **Jumper Wires** (to connect everything without soldering).



---

## 2. "Wiring Guide" (Connection)

This is the most important part! You have to plug the right wire into the right hole. Here is the exact plan from our `config.h` file.

| Component Pin | Connects To → | ESP32 Pin |
| :--- | :--- | :--- |
| **OLED Display** | | |
| VCC (Power) | → | **3.3V** |
| GND (Ground) | → | **GND** |
| SCL (Clock) | → | **D22** |
| SDA (Data) | → | **D21** |
| | | |
| **Touch Sensor (TTP223)** | | |
| VCC (Power) | → | **3.3V** |
| GND (Ground) | → | **GND** |
| SIG (Signal) | → | **D13** |
| | | |
| **Buzzer** | | |
| `+` (Positive) | → | **D25** |
| `-` (Negative) | → | **GND** |


## Diagram

![App Screenshot](https://github.com/pstarz7/Shiro-Firmware-v1/blob/7cdd32b3825aa4fad794096a79f0a9f7eeb22d8a/SHIRO%20-%20robot%20diagram.png)


**⭐ Super Important Note!**
Everything must share the same "Ground." This means the `GND` pin from the ESP32, the `GND` from the OLED, the `GND` from the TTP223, and the `-` pin from the Buzzer must all be connected together.

---

## 3. "Setup Guide" (Software)

This guide will get the code from here onto your ESP32 brain.

### Step 1: Get the Main Tool (Arduino IDE)
* Download and install the free Arduino IDE from the [official website](https://www.arduino.cc/en/software).

### Step 2: Teach Arduino IDE about Your ESP32
* In the Arduino IDE, go to `File` > `Preferences`.
* Find the box called "Additional Boards Manager URLs".
* Paste this exact link into the box:
    ```
    [https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json](https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json)
    ```
* Click OK.
* Now, go to `Tools` > `Board` > `Boards Manager...`
* Search for **"esp32"**.
* Click "Install" on the one by Espressif Systems.

### Step 3: Get the "Skills" (Libraries)
Shiro needs special libraries to know how to work.
* Go to `Tools` > `Manage Libraries...`
* Search for and **install** these libraries, one by one:
    1.  `Adafruit GFX Library`
    2.  `Adafruit SSD1306`
    3.  `ChronosESP32` (When you install this, it will ask to also install `NimBLE-Arduino`. Click **"Install All"**.)

### Step 4: Get this Code
* Click the green **`< > Code`** button at the top of this GitHub page.
* Choose **"Download ZIP"**.
* Unzip the folder. It will be named `Shiro_Firmware-main` or something similar.
* Open the `Shiro_v7_EmotionEngine` folder inside, and double-click the `Shiro_v7_EmotionEngine.ino` file to open it in the Arduino IDE.

### Step 5: Get Your Phone Ready
* Shiro gets its time, notifications, and weather from a free app called **Chronos**.
* Go to the App Store or Play Store and install the "Chronos" app.
* Open the app, go to the Weather settings, and **set your location** (like "Berhampur"). This is very important!

### Step 6: Upload!
* Plug your ESP32 into your computer with a USB cable.
* In the Arduino IDE, go to `Tools` > `Board` and select your board (e.g., "ESP32 Dev Module").
* Go to `Tools` > `Port` and select the COM port that your ESP32 is on.
* Click the **Upload** button (the arrow pointing right).
* Wait for it to finish. Your Shiro should now be alive!

---

## 4. "How to Play" (User Manual)

The controls are simple and change based on what screen you are on.

#### On the Animation Screen (Shiro's Face)
* **Single-Tap:** Wakes Shiro up if it's sleeping.
* **Double-Tap:** Opens the Time Screen.
* **Triple-Tap:** "Feeds" Shiro. This plays the `foody.h` animation and stops it from being hungry.
* **Hold (Rub):** This is for petting!
    * **1-3 Rubs:** Shiro is happy and plays the `love.h` animation.
    * **4-5 Rubs:** Shiro gets annoyed and plays the `frustrated.h` animation.
    * **6+ Rubs:** Shiro gets angry! It plays the `angry.h` animation.
    * *(If you wait 10 seconds, Shiro calms down and the rub count resets.)*

#### On the Utility Screens (Time, Weather, etc.)
* **Single-Tap:**
    * On the **Find Phone** screen: Toggles the ringer on/off.
    * On **all other** screens: **Dismisses** the screen and goes back to the animation.
* **Double-Tap:** **Cycles** through the pages (Time → Weather → Find Phone → Time...)

Enjoy your new desk friend!
