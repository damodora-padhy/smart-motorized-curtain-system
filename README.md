# Smart Motorized Curtain System 🪟

An ESP32-based smart motorized curtain system developed during my internship at **NIT Rourkela**.

The system automates curtain opening and closing using environmental and obstacle sensors while also providing manual control through **Bluetooth** and **Blynk IoT over Wi-Fi**.

## 🏆 Achievement

🥈 **Second Prize** — Project developed during my internship at **NIT Rourkela**.

## 📌 Project Overview

The system uses two stepper motors to operate the curtain simultaneously.

When the curtain opens:
- Motor A rotates clockwise.
- Motor B rotates anticlockwise.

When the curtain closes:
- Motor A rotates anticlockwise.
- Motor B rotates clockwise.

This synchronized opposite-direction movement allows the two sides of the curtain to open and close together.

## ⚙️ Main Components

- ESP32 DevKit
- 2 × 28BYJ-48 Stepper Motors
- 2 × ULN2003 Stepper Motor Driver Modules
- FC-37 Rain Sensor
- LDR Light Sensor
- IR Obstacle Sensor
- DHT11 Temperature & Humidity Sensor
- 5V DC Power Supply
- Bluetooth
- Wi-Fi
- Blynk IoT

## 🧠 Automatic Operation

The curtain can operate automatically according to sensor conditions.

### ☀️ Day / Night Control
- During daylight, the curtain opens.
- During nighttime, the curtain closes.

### 🌧️ Rain Detection
When rain is detected, the curtain closes to protect the area behind the curtain.

### 🚨 IR Obstacle Detection
When an object is detected by the IR sensor, the curtain opens.

### 🌡️ Temperature Monitoring
The DHT11 sensor is used to monitor temperature and humidity and can contribute to the automatic curtain control.

## 🎮 Control Modes

The system provides different control options through the Bluetooth Serial Terminal and Blynk IoT.

### Bluetooth Control
The Bluetooth Serial Terminal can be used for:
- Manual Open
- Manual Close
- Sensor Status
- Auto Mode
- Emergency Stop / Close

### BlynK IoT Control
The Blynk IoT application provides:
- Open control
- Close control
- Auto Mode
- System status
- Temperature monitoring

## 📡 Communication

The ESP32 provides two wireless communication methods:

**Bluetooth**
→ Bluetooth Serial Terminal App

**Wi-Fi**
→ Blynk IoT App

This allows the curtain to be controlled and monitored wirelessly.

## 🔧 System Architecture

Sensors
→ ESP32 DevKit
→ ULN2003 Motor Drivers
→ Dual Stepper Motors
→ Motorized Curtain

The ESP32 processes sensor inputs and controls both stepper motors according to the selected operating mode.

## 📂 Project Structure

```text
smart-motorized-curtain-system/
│
├── README.md
├── src/
├── diagrams/
├── images/
└── documentation/
