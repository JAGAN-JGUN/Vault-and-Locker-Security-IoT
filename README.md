# 🔐 Vault and Locker Security IoT System

This project implements an **IoT-based vault and locker security system** using an ESP32 microcontroller. The system monitors IR sensors for intrusion detection and controls lock actuators. It also sends **real-time email alerts** when unauthorized access is detected and can receive email-based commands.

---

## 📦 Features

- 🔍 **Intrusion Detection** via dual IR sensors
- 📧 **Email Alerts** using Gmail SMTP
- 📥 **Remote Command Retrieval** via IMAP
- ⏰ **Real-Time Clock** with NTP synchronization
- 🔒 Controls 3 outputs (`L1`, `L2`, `L3`) for actuating lockers or alarms

---

## 🛠️ Hardware Requirements

- ESP32 microcontroller
- 2× IR sensors (for motion or door detection)
- Locking mechanism or actuators (connected to L1, L2, L3 pins)
- Wi-Fi connection

---

## 💻 Software & Libraries

- [ESP_Mail_Client](https://github.com/mobizt/ESP-Mail-Client)
- WiFi.h (ESP32 core)
- Arduino core for ESP32
- Gmail SMTP and IMAP

---

## 🔧 Pin Configuration

| Component       | ESP32 Pin |
|----------------|-----------|
| IR Sensor A     | 13        |
| IR Sensor B     | 14        |
| Lock Output 1 (L1) | 15     |
| Lock Output 2 (L2) | 4      |
| Lock Output 3 (L3) | 5      |

---

## 📧 Email Setup

In the code, replace:

```cpp
#define WIFI_SSID "Your Wifi Name"
#define WIFI_PASSWORD "Your Wifi Pwd"

#define AUTHOR_EMAIL "your_esp_email@gmail.com"
#define AUTHOR_PASSWORD "your_app_password"

#define RECIPIENT_EMAIL "recipient_email@gmail.com"
