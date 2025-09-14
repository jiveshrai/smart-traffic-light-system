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
![screenshot](docs/demo.png)

## Microcontroller
- **Manufacturer:** Microchip Technology Inc.
- **Model:** ATmega328P (Arduino Uno R3)

---
*Developed and simulated for educational purposes.*
