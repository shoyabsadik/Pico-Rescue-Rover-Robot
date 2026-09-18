<div align="center">

# 🔥 Pico Rescue Rover

### Smart Multi-Mode Firefighting & Rescue Robot

**Developed by [M. Shoyab Sadik](https://shoyabsadik.github.io/)**
Founder & CEO, Pico Robotics · B.Sc. in Electrical & Electronic Engineering (EEE), IUBAT

**Pico Robotics** — *"From Code to Creation"*

![Platform](https://img.shields.io/badge/Platform-Arduino-00979D?logo=arduino&logoColor=white)
![Language](https://img.shields.io/badge/Language-C%2B%2B-blue)
![Status](https://img.shields.io/badge/Status-Active-brightgreen)
![License](https://img.shields.io/badge/License-See%20Below-lightgrey)

</div>

---

## Table of Contents

- [Project Overview](#project-overview)
- [Project Features](#project-features)
- [Working Modes](#working-modes)
- [Hardware Components](#hardware-components)
- [Pin Configuration](#pin-configuration)
- [System Workflow](#system-workflow)
- [Firefighting Operation](#firefighting-operation)
- [Obstacle Avoidance](#obstacle-avoidance)
- [Hand Following](#hand-following)
- [Bluetooth Command Reference](#bluetooth-command-reference)
- [LCD Display](#lcd-display)
- [Software & Libraries](#software--libraries)
- [Circuit Diagram](#circuit-diagram)
- [Project Images](#project-images)
- [Future Improvements](#future-improvements)
- [Project Applications](#project-applications)
- [Limitations](#limitations)
- [Safety Notice](#safety-notice)
- [Author](#author)
- [Pico Robotics](#pico-robotics)
- [License](#license)

---

## Project Overview

**Pico Rescue Rover** is a multifunctional Arduino-based robot designed for firefighting and rescue-assistance applications. It combines manual Bluetooth remote control with autonomous behaviors — automatic flame detection and suppression, ultrasonic obstacle avoidance, and hand-following navigation — into a single platform. The rover is built around an **Arduino Nano**, driven by an **L298N motor driver**, and equipped with a flame sensor, ultrasonic distance sensor, servo-actuated nozzle, water pump, buzzer, soil moisture sensor, and a 16x2 I2C LCD for real-time status feedback.

The project demonstrates practical integration of embedded control, sensor fusion, and actuator management on a resource-constrained microcontroller, making it suitable for academic demonstration, robotics competitions, and further research into autonomous rescue systems.

---

## Project Features

- Bluetooth-controlled RC mode (serial command based)
- Forward, reverse, left, right, and pivot-style diagonal movement
- Variable motor speed control via serial command
- Automatic flame detection using a digital flame sensor
- Automatic firefighting response using a water pump
- Servo-controlled water nozzle positioning
- Ultrasonic obstacle detection (HC-SR04)
- Automatic obstacle avoidance (reverse-and-turn maneuver)
- Hand-following mode using ultrasonic distance measurement
- Soil moisture monitoring, displayed live on the LCD
- Buzzer alert during fire response
- 16x2 I2C LCD for mode and status display

---

## Working Modes

The rover operates in one of three selectable modes, with the firefighting system running as a higher-priority background process at all times.

| Mode | Description |
|---|---|
| **RC Mode** | Default mode. The rover responds directly to Bluetooth movement and speed commands. |
| **Obstacle Avoidance Mode** | The rover drives forward autonomously, reversing and turning right whenever the ultrasonic sensor detects an object closer than the configured threshold. |
| **Hand Follow Mode** | The rover uses ultrasonic distance readings to follow a hand or object — moving forward when far, holding position at a set range, and reversing if it gets too close. |
| **Firefighting System** | Runs continuously in the background regardless of the active mode. When the flame sensor detects fire, it automatically overrides current movement to approach, alert, and spray water at the fire source. |

---

## Hardware Components

| Component | Quantity |
|---|---|
| Arduino Nano | 1 |
| L298N Motor Driver | 1 |
| DC Gear Motor | 2 |
| Flame Sensor | 1 |
| HC-SR04 Ultrasonic Sensor | 1 |
| Soil Moisture Sensor | 1 |
| SG90 Servo Motor | 1 |
| Mini Water Pump | 1 |
| 1-Channel Relay Module | 1 |
| 16x2 I2C LCD | 1 |
| Buzzer | 1 |
| 18650 Li-ion Battery | 2 |
| Robot Chassis & Wheels | 1 set |

---

## Pin Configuration

Pin assignments below are taken directly from the `#define` statements in the source code.

| Function | Arduino Pin | Type |
|---|---|---|
| Left Motor IN1 | D7 | Digital Output |
| Left Motor IN2 | D8 | Digital Output |
| Left Motor Enable (PWM speed) | D5 | PWM Output |
| Right Motor IN1 | D9 | Digital Output |
| Right Motor IN2 | D10 | Digital Output |
| Right Motor Enable (PWM speed) | D6 | PWM Output |
| Flame Sensor | D4 | Digital Input |
| Soil Moisture Sensor | A0 | Analog Input |
| Ultrasonic Trigger (HC-SR04) | D11 | Digital Output |
| Ultrasonic Echo (HC-SR04) | D12 | Digital Input |
| Water Pump (via Relay) | D2 | Digital Output |
| Buzzer | D13 | Digital Output |
| Nozzle Servo | D3 | PWM Output |

> **Note:** The 16x2 I2C LCD (address `0x27`) and the Bluetooth module communicate over the Arduino Nano's hardware I2C (A4/SDA, A5/SCL) and hardware Serial (D0/D1) lines respectively. These are not explicit `#define` pins in the code but are fixed by the Nano's hardware peripherals.

---

## System Workflow

1. On startup, the Arduino initializes all motor, sensor, and actuator pins, attaches the nozzle servo to its rest position, initializes the I2C LCD, and starts the serial connection for Bluetooth communication at **9600 baud**.
2. The main loop continuously:
   - Updates the soil moisture reading every 300 ms.
   - Checks the flame sensor; if fire is detected and no fire response is already active, it triggers the automatic firefighting sequence.
   - Processes the firefighting sequence state machine, if active.
   - Reads and executes any incoming Bluetooth serial commands.
   - If a firefighting response is in progress, all other mode logic is skipped for that loop cycle.
   - Otherwise, runs the logic for the currently active mode — Obstacle Avoidance or Hand Follow (RC Mode responds directly to Bluetooth commands with no additional polling logic).
3. The LCD is updated on every state change to reflect the current mode, distance readings, soil moisture percentage, and system status.

---

## Firefighting Operation

The firefighting system runs automatically and takes priority over the active driving mode.

1. **Flame Detection** — The digital flame sensor reads `LOW` when fire is detected, triggering the response.
2. **Move Toward Fire** — The rover drives forward at the configured firefighting speed for a short, fixed duration (800 ms).
3. **Stop & Alert** — The rover stops, and the buzzer is activated to signal a fire response in progress.
4. **Water Pump Activation** — The relay-driven water pump is switched on.
5. **Servo Nozzle Positioning** — The nozzle servo moves to its firefighting angle to aim the water spray.
6. **Water Spraying** — The pump continues spraying for the configured duration (approximately 3.3 seconds combined).
7. **Sequence End** — The pump and buzzer are switched off, the servo returns to its rest position, motor speed is restored to its pre-fire value, and the system displays **"SYSTEM READY"** on the LCD.

---

## Obstacle Avoidance

When Obstacle Avoidance Mode is active:

1. The rover drives forward while continuously polling the ultrasonic sensor.
2. If the measured distance falls below the configured obstacle threshold (20 cm), the rover stops and reverses for a fixed duration (300 ms).
3. After reversing, the rover turns right for a fixed duration (500 ms) to clear the obstacle.
4. The rover then resumes forward motion and distance monitoring.
5. If the ultrasonic sensor returns an invalid reading (sensor timeout), the rover stops and displays a sensor error on the LCD.

---

## Hand Following

When Hand Follow Mode is active, the rover reacts to ultrasonic distance readings from an object (typically a hand) in front of it:

| Distance Condition | Rover Behavior |
|---|---|
| Greater than the far threshold (15 cm) | Moves forward |
| Between the near and far thresholds (8–15 cm) | Holds position (stops) |
| Less than the near threshold (8 cm) | Reverses |
| Invalid sensor reading | Stops and displays a sensor error |

---

## Bluetooth Command Reference

Commands are sent as single ASCII characters over Serial at 9600 baud (compatible with standard HC-05/HC-06 Bluetooth modules).

| Command | Action |
|---|---|
| `F` | Move forward |
| `B` | Move reverse |
| `L` | Turn left |
| `R` | Turn right |
| `I` | Forward-right pivot |
| `G` | Forward-left pivot |
| `J` | Reverse-right pivot |
| `H` | Reverse-left pivot |
| `S` | Stop |
| `W` | Enable Obstacle Avoidance Mode |
| `w` | Disable Obstacle Avoidance Mode (return to RC) |
| `X` | Enable Hand Follow Mode |
| `x` | Disable Hand Follow Mode (return to RC) |
| `0`–`9` | Set motor speed (mapped to a range of 80–245) |
| `q` | Set maximum motor speed (255) |

---

## LCD Display

The 16x2 I2C LCD shows the current mode with live soil moisture percentage on the top row, and a status message on the bottom row. Example displays:

```
RC       S: 45%
FORWARD
```

```
OBSTACLE S: 45%
D:25.4
```

```
OBSTACLE S: 45%
REVERSING
```

```
HAND     S: 45%
FORWARD 12.5cm
```

```
FIRE     S: 45%
SPRAYING WATER
```

---

## Software & Libraries

- [`Servo.h`](https://www.arduino.cc/reference/en/libraries/servo/) — Controls the nozzle servo motor
- [`Wire.h`](https://www.arduino.cc/reference/en/language/functions/communication/wire/) — I2C communication for the LCD
- [`LiquidCrystal_I2C.h`](https://github.com/johnrickman/LiquidCrystal_I2C) — 16x2 I2C LCD driver

---

## Circuit Diagram

![Circuit Diagram](images/Diagram.png)

---

## Project Images

| Front View | Side View | Circuit Diagram |
|---|---|---|
| ![Robot Front](images/Real_Picture.png) | ![Robot Side](images/robot-side.jpg) | ![Circuit Diagram](images/circuit-diagram.png) |

---

## Future Improvements

- ESP32-based wireless control (Wi-Fi / app-based control replacing Bluetooth)
- Onboard camera / FPV live video feed
- GPS-based location tracking for rescue coordination
- GSM/IoT-based fire alerts to a remote server or mobile app
- More advanced autonomous navigation (SLAM or sensor-fusion based)
- Improved fire localization using multiple flame sensors or a thermal camera

---

## Project Applications

- Educational demonstration of embedded systems and robotics integration
- Robotics competitions and technical exhibitions
- Prototype platform for rescue-robotics research
- Base platform for further IoT/automation-based fire-safety projects

---

## Limitations

- Single flame sensor provides limited fire-direction accuracy.
- Fixed-duration movement and avoidance timings are not adaptive to varying terrain or obstacle sizes.
- Water reservoir and pump capacity are limited by the small onboard water tank.
- Bluetooth range and reliability depend on the specific module and environment.
- No autonomous path planning or mapping capability in the current implementation.

---

## Safety Notice

> This robot is a **prototype developed for educational and research purposes only**. It is **not** certified or intended for use as a professional firefighting or life-safety system. Do not rely on this device in real emergency situations involving actual fire hazards or human safety.

---

## Author

**M. Shoyab Sadik**
Founder & CEO, Pico Robotics
B.Sc. in Electrical & Electronic Engineering (EEE), IUBAT

- 🌐 Website: [shoyabsadik.github.io](https://shoyabsadik.github.io/)
- 💼 LinkedIn: [linkedin.com/in/shoyabsadik](https://www.linkedin.com/in/shoyabsadik/)
- 💻 GitHub: [github.com/shoyabsadik](https://github.com/shoyabsadik)

---

## Pico Robotics

**Pico Robotics**
*"From Code to Creation"*

---

## License

*License not yet specified. Add your preferred license (e.g., MIT, Apache 2.0) here.*

