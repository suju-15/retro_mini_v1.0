# Pinout & Wiring Guide (ESP32-C3 SuperMini)

This document details hardware pin mappings for the **Retro Mini Console** running on an **ESP32-C3 SuperMini** board powered by a single-cell LiPo battery.

---

## 1. Hardware Mapping

| Component | Pin Function | ESP32-C3 SuperMini Pin | Wiring Details |
| :--- | :--- | :--- | :--- |
| **OLED Display** | SDA (Data) | `GPIO 8` | Native Hardware I2C SDA |
| **OLED Display** | SCL (Clock) | `GPIO 9` | Native Hardware I2C SCL |
| **D-Pad UP** | Button Input | `GPIO 1` | Connect to GND when pressed (`INPUT_PULLUP`) |
| **D-Pad DOWN** | Button Input | `GPIO 2` | Connect to GND when pressed (`INPUT_PULLUP`) |
| **D-Pad LEFT** | Button Input | `GPIO 3` | Connect to GND when pressed (`INPUT_PULLUP`) |
| **D-Pad RIGHT** | Button Input | `GPIO 4` | Connect to GND when pressed (`INPUT_PULLUP`) |
| **Action BTN A** | Select / Action | `GPIO 5` | Connect to GND when pressed (`INPUT_PULLUP`) |
| **Haptic Motor** | Vibration Drive | `GPIO 6` | Drive via NPN Transistor / MOSFET |
| **Power Input** | LiPo Positive (+) | `5V Pin` | **Boosted LiPo output (~5V)** to internal LDO |
| **Power Ground**| LiPo Negative (-) | `GND Pin` | System Common Ground |

---

## 2. Wiring & Power Setup Notes

1. **LiPo Battery Connection:**
   - Connect your LiPo battery output (preferably boosted to 5V using a TP4056 + 5V boost circuit, or a LiPo shield) directly to the **5V** and **GND** pins on the ESP32-C3 SuperMini.
   - *Note:* Feeding standard 3.7V LiPo power into the 5V pin uses the board's on-board LDO regulator to safely drop down to 3.3V for the ESP32-C3 chip and OLED display.

2. **Button Connections:**
   - Wire one side of each tactile button to its respective GPIO pin (`1`, `2`, `3`, `4`, `5`).
   - Wire the opposing side of all buttons to a shared **GND** wire.
   - External pull-up resistors are **not required** as internal software pull-ups are enabled in the code.

3. **Vibration Motor Driver:**
   - **Do NOT drive the haptic motor directly from GPIO 6.** High current draw can permanently damage the ESP32-C3 chip.
   - Use an NPN transistor (e.g., 2N2222) with a $1\text{k}\Omega$ resistor on the base connected to GPIO 6, and a flyback diode (1N4148) across the motor leads.
