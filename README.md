# Smart Irrigation System By Call Sms

This is a smart irrigation system that uses a soil moisture sensor and a GSM module (SIM900A) to automate irrigation and send SMS alerts. The system controls a water pump through a relay based on soil conditions, aiming to conserve water and reduce manual effort.

## Features

- Automatically detects soil moisture level
- Turns on/off water pump using relay
- Sends SMS notification via SIM900A GSM module when pump is activated
- Optional temperature sensor (currently non-functional/tested)

## Components Used

- **Microcontroller**: C-compatible (e.g., Arduino UNO)
- **GSM Module**: SIM900A
- **Soil Moisture Sensor**
- **Relay Module**
- **Water Pump**
- **Temperature Sensor (Optional)**

## How It Works

1. The soil moisture sensor reads the soil's moisture level.
2. If moisture is below a defined threshold:
   - The microcontroller activates the water pump via relay.
   - An SMS alert is sent using the SIM900A GSM module.
3. Once adequate moisture is detected, the pump is turned off.

## Technologies Used

- Programming Language: **C**
- Hardware: Basic sensors and microcontroller modules

## Wiring and Setup

1. Connect soil moisture sensor to analog input.
2. Relay module to digital output (for pump control).
3. SIM900A GSM module to serial communication pins (TX/RX).
4. Upload the `smart_irrigation.c` code to the microcontroller.
5. Insert SIM card into GSM module and power the system.

> *Note: Temperature sensor is included but not active in this version.*

## Folder Structure
