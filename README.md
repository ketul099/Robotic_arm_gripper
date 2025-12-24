

# 🦾 Robotic Arm Gripper

### Real-Time Finger Motion Control Using Flex-Sensor Glove & nRF24L01

This project implements a **real-time wearable human–machine interface** that captures **actual finger movement** using a **flex-sensor glove** and wirelessly controls a **high-duty mechanical robotic arm gripper**.

By wearing the glove, the user’s **natural finger bending motion** is transmitted wirelessly and reproduced by **servo-driven mechanical fingers**, enabling **intuitive, proportional, and responsive grasp control**.

---

## 🎯 Project Objective

To design a **low-latency, wireless, and scalable control system** that:

* Accurately tracks **human finger movement**
* Transmits data **wirelessly and reliably**
* Controls a **mechanical arm/gripper capable of high-duty gripping**
* Ensures **safe operation** through calibration, filtering, and failsafe logic

---

## 🧠 System Overview

### Human → Machine Control Flow

1. **Flex-Sensor Glove (Transmitter)**

   * 5 flex sensors capture real finger bending
   * Signals are filtered and calibrated
   * Data is normalized and transmitted wirelessly

2. **Wireless Communication**

   * nRF24L01 transceivers
   * Low-latency, bidirectional RF link
   * Automatic retries and link recovery

3. **Robotic Arm Receiver**

   * Received finger motion mapped to servo angles
   * Each servo controls one mechanical finger
   * High-duty gripping supported via external power

---

## 🧩 Features

✔ Real-time finger motion replication
✔ Wireless control using **nRF24L01**
✔ 5-finger independent control
✔ Calibration mode for accurate mapping
✔ Signal smoothing and noise rejection
✔ Fail-safe mode on link loss
✔ Suitable for **high-duty mechanical grippers**
✔ Modular & expandable design

---

## 🛠 Hardware Requirements

### Electronics

* 2 × Arduino (UNO / Nano recommended)
* 2 × nRF24L01 modules (**with capacitor**)
* 5 × Flex sensors
* 5 × Resistors (10kΩ–47kΩ for voltage dividers)
* 5 × Servo motors (SG90 / MG996R / industrial servos)
* External **5V–6V high-current power supply** (2–5A)
* Push button (calibration)
* Status LED + 220Ω resistor

### Power Notes (Very Important)

⚠ **nRF24L01 → 3.3V ONLY**
⚠ Add **10µF–100µF capacitor** across VCC–GND near nRF
⚠ Servos must use **external power**, Arduino GND must be common

---

## 🔌 Wiring Summary

### Flex Sensor Glove (TX)

* Flex Sensors → A0–A4 (voltage divider)
* Calibration Button → D2 → GND
* Status LED → D3 → 220Ω → GND
* nRF24 CE → D9
* nRF24 CSN → D10

### Robotic Arm (RX)

* Servo Signal Pins → D3, D4, D5, D6, D7
* nRF24 CE → D8
* nRF24 CSN → D10
* Servo Power → External 5V (common GND)

---

## 📂 Repository Structure

```
Robotic_arm_gripper/
│
├── flex_tx/        # Flex-sensor glove (Transmitter)
│   └── flex_tx.ino
│
├── servo_rx/       # Robotic arm controller (Receiver)
│   └── servo_rx.ino
│
└── README.md
```

---

## 🚀 How It Works

### Transmitter (Flex Glove)

* Reads analog flex values
* Applies moving-average smoothing
* Performs calibration (min/max per finger)
* Normalizes values (0–1000)
* Transmits data at ~50Hz

### Receiver (Robotic Arm)

* Receives normalized finger motion
* Maps values to servo angles
* Drives mechanical fingers proportionally
* Activates failsafe if RF link is lost

---

## 🎚 Calibration Procedure

1. Power ON the glove (TX)
2. Press and hold **Calibration Button**
3. Move fingers through full bend & release
4. Release button to save calibration
5. Robotic arm now mirrors real finger motion

---

## 🧯 Safety & Failsafe

* Automatic neutral position on RF link loss
* Limited servo angle ranges to protect mechanics
* Filtering prevents sudden jumps
* Link monitoring and recovery logic

---

## 🔬 Applications

* Human-robot interaction (HRI)
* Teleoperation systems
* Prosthetic control research
* Industrial robotic gripping
* Rehabilitation & assistive devices
* Educational robotics projects

---

## 📈 Future Improvements

* Force feedback (FSR / current sensing)
* Bidirectional haptic feedback
* ROS / ROS2 integration
* Encoder-based servo feedback
* Machine-learning-based gesture recognition
* Industrial actuator upgrade

---

## 👨‍💻 Author

**Ketul Patel**
Embedded Systems Engineer
Specialized in Embedded AI, Robotics & Human–Machine Interfaces

---

