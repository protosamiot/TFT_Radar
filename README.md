# 📡 Ultrasonic Radar System using Arduino Nano + 1.8" TFT (ST7735)

🚀 A live **Ultrasonic Radar System** built using **Arduino Nano**, **HC-SR04 ultrasonic sensor**, **1.8" SPI TFT display (ST7735)** and a **buzzer**.  
This project displays a **semi-circle radar UI** with a sweeping scan line and shows detected objects as a **red dot** in real-time.

⭐ If you like this project, consider giving it a star!

---

## 📌 Features
✅ Semi-circle radar display UI  
✅ Live scan sweep animation  
✅ Real-time ultrasonic distance measurement  
✅ Maximum radar range: **50 cm**  
✅ Object shown as a **red dot**  
✅ Danger alert at **<= 15 cm**  
🔴 Red dot blinks + 🔊 buzzer ON  

---

## 🧰 Components Required
- Arduino Nano
- 1.8" TFT SPI Display (ST7735 - 128x160)
- HC-SR04 Ultrasonic Sensor
- Buzzer (Active or Passive)
- Breadboard
- Jumper Wires
- USB Cable (for Arduino Nano)

---

## 🔌 Connections

### 📺 TFT Display (ST7735 SPI - 8 Pin)
| TFT Pin | Arduino Nano |
|--------|--------------|
| VCC | 5V (or 3.3V if your module requires) |
| GND | GND |
| CS | D10 |
| DC / A0 | D8 |
| RST | D9 |
| SDA / MOSI | D11 |
| SCK / SCL | D13 |
| LED / BL | 5V |

---

### 📡 HC-SR04 Ultrasonic Sensor
| HC-SR04 Pin | Arduino Nano |
|------------|--------------|
| VCC | 5V |
| GND | GND |
| TRIG | D6 |
| ECHO | D7 |

---

### 🔊 Buzzer
| Buzzer Pin | Arduino Nano |
|-----------|--------------|
| + (Positive) | D5 |
| - (Negative) | GND |

---

## 📦 Libraries Required
Install these libraries from **Arduino Library Manager**:

- **Adafruit GFX Library**
- **Adafruit ST7735 and ST7789 Library**

---

## ⚙️ How It Works
1. The TFT display shows a **semi-circle radar** with range arcs.
2. The scan line sweeps from **180° to 360°**.
3. The ultrasonic sensor measures the distance in real-time.
4. If an object is within **50 cm**, a **red dot** appears on the radar.
5. If the object is **closer than 15 cm**, the dot blinks and the buzzer turns ON 🚨.

---

## 🎥 Video Tutorial (YouTube)
📌 Watch the project here:  
👉 https://www.youtube.com/@protosamiot

---

## 💻 Author
**ProtoSam IoT**  
🔗 GitHub: https://github.com/ProtoSamIoT  
📺 YouTube: https://www.youtube.com/@protosamiot  

---

## ⭐ Support
If you found this useful:
- ⭐ Star this repository
- 🔔 Subscribe on YouTube
- 🛠️ Share with your friends

---

## 📜 License
This project is open-source and free to use for learning and educational purposes.
