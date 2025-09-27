# 💧 Smart GSM Irrigation System

An Arduino-based smart farming project designed to help farmers remotely control and monitor their agricultural water pump using SMS and phone calls. The system also supports a fully automatic irrigation mode based on real-time soil moisture data.

---


## ✨ Features

- **Remote Control:** Turn the water pump on/off from anywhere using simple SMS commands or by making a phone call from an authorized number.
- **Automatic Mode:** Automatically activates the pump when the soil moisture drops below a pre-configured threshold.
- **Crop-Specific Profiles:** Comes with pre-configured, optimal moisture thresholds for different crops like Wheat, Rice, Maize, and Potato, which can be selected via SMS.
- **Live Status Reports:** Get a complete system report—including motor status, current soil moisture level, auto-mode status, and more—with a single `status` command.
- **Dynamic Configuration:** Adjust the moisture threshold and the automatic check interval remotely via SMS without needing to reprogram the device.
- **Multi-User Security:** The system is secure and only responds to authorized phone numbers. New numbers can be added or removed remotely by an admin user.
- **Smart Sensing:** To ensure accuracy and avoid false readings, the system takes an average of 10 sensor readings before making a decision.
- **Robust & Reliable:** Implements a hardware watchdog timer to automatically restart the system in case of a software freeze, ensuring long-term reliability.

---

## 🛠️ Hardware Required

* Arduino UNO/Nano
* SIM800L / SIM900A GSM Module
* Capacitive Soil Moisture Sensor V1.2
* 5V Single Channel Relay Module
* Water Pump (Motor)
* Jumper Wires & Power Supply

---

## 💻 Software & Libraries

* **Arduino IDE**
* **Libraries:**
    * `<SoftwareSerial.h>`
    * `<avr/wdt.h>`

---

## 🚀 How to Use

Control the system by sending the following SMS commands from an authorized mobile number to the SIM card in the GSM module.

### SMS Commands
- `status` - To get a full report of the system's current status.
- `chal` - To manually turn the motor ON.
- `band` - To manually turn the motor OFF.
- `ai on` - To enable the automatic irrigation mode.
- `ai off` - To disable the automatic irrigation mode.
- `crop: <crop_name>` - To set the threshold based on a crop profile (e.g., `crop: rice`).
- `set threshold: <value>` - To manually set a custom moisture threshold (e.g., `set threshold: 650`).
- `set interval: <seconds>` - To set the time interval for checks in auto mode (e.g., `set interval: 300`).
- `add auth: <+91...number>` - To add a new authorized number.
- `remove auth: <+91...number>` - To remove an existing authorized number.

---
