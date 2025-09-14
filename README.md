# Smart Traffic Light System (Simulated)

This project simulates a smart traffic light system using **Arduino Uno (ATmega328P microcontroller)** in the [Wokwi online emulator](https://wokwi.com).

## Features
- Adaptive timing with virtual IR sensors (simulated by pushbuttons).
- Traffic lights for two lanes (Red/Yellow/Green).
- Data logging to EEPROM for counting vehicles per cycle.
- Runs fully in an online emulator (no hardware needed).

## How to Run
1. Open [Wokwi](https://wokwi.com/projects/new/arduino-uno).
2. Copy `sketch.ino` code into `sketch.ino`.
3. Copy `wokwi-project.json` into a file named `wokwi-project.json`.
4. Click **Start Simulation**.
5. Open the **Serial Monitor** to view logs.
6. Press pushbuttons in the emulator to simulate vehicle detection.

## Demo
<img width="1152" height="780" alt="image" src="https://github.com/user-attachments/assets/e845ab83-099c-482f-9ff4-36e7b3d6cbe4" />

## Microcontroller
- **Manufacturer:** Microchip Technology Inc.
- **Model:** ATmega328P (Arduino Uno R3)

---
*Developed and simulated for educational purposes.*
