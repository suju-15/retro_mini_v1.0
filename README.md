# Retro Mini - Multi-Game Handheld Console (ESP32-C3 SuperMini)

A compact 8-bit retro gaming console built around the ultra-small **ESP32-C3 SuperMini** board and a **0.96" SSD1306 OLED display**.

---

## Technical Specifications

- **Microcontroller:** ESP32-C3 SuperMini (RISC-V Single-Core @ 160MHz)
- **Display:** 0.96" SSD1306 OLED (128x64 resolution, I2C interface)
- **Power Source:** Single-Cell 3.7V LiPo Battery connected via 5V pin
- **Feedback:** Haptic vibration motor for game interactions

---

## Included Games

1. **Snake** - Classic grid navigation with growth mechanics.
2. **Pong** - Single-player paddle vs. automated opponent.
3. **Tetris** - Custom board grid, piece rotation, and row-clear scoring.
4. **Space Dodge** - Top-down shooter with laser firing (**Button A**).
5. **Flappy Bird** - Tap-to-fly physics with scrolling obstacle columns.

---

## Controls & Shortcuts

- **Navigation / Movement:** D-Pad (UP, DOWN, LEFT, RIGHT)
- **Primary Action (Select / Shoot / Jump / Rotate):** Action Button A
- **Emergency In-Game Exit:** **Hold LEFT + RIGHT simultaneously for 1 second** to force exit back to the Game Selection Menu.

---

## Arduino IDE Board Configuration

1. Install the ESP32 board package in Arduino IDE (`https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`).
2. Go to **Tools > Board** and select **ESP32C3 Dev Module** (or **ESP32-C3 SuperMini** if available).
3. Set parameters:
   - **USB CDC On Boot:** *Enabled*
   - **CPU Frequency:** *160MHz (WiFi/BT off)*
   - **Flash Size:** *4MB*
4. Connect via USB-C and flash the project code.
