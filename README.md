# 🤖 Line Follower Analog dan Digital

<p align="center">
  <img src="Dokumentasi/Start%20Merah.gif" width="32%" />
  <img src="Dokumentasi/Start%20Kuning.gif" width="32%" />
  <img src="Dokumentasi/LF%20Analog.gif" width="32%" />
</p>

The **Line Follower Analog dan Digital** is a line-following robot developed with **two independent control modes**: a **Digital** mode driven by ESP32-based PID algorithm, and an **Analog** mode built entirely from discrete op-amp (LM358) comparator/PID circuitry. Developed for PID control research and robotics competition.

## 🚀 Project Goal

Enable the robot to:
- Follow a black line track accurately using either digital or analog control.
- Handle intersections, dashed lines, and FINISH zones reliably.
- Compare performance and response characteristics between digital PID and analog PID control.

---

## 🧩 System Overview

### 🔧 Hardware Structure

#### 1. **Sensor Array**
- **8-channel Photodiode/IR array** for line detection.
- **Multiplexer (MUX)** for efficient analog input switching to the microcontroller.

#### 2. **Digital Control Unit**
- **ESP32** – central microcontroller running the PID algorithm.
- Auto-calibration for black/white line threshold (`blackMode()` / `whiteMode()`).
- Junction, dashed-line, and FINISH zone detection logic.

#### 3. **Analog Control Unit**
- **Op-amp LM358** – comparator and PID error-correction circuitry.
- Discrete analog components for real-time steering correction without a microcontroller.

#### 4. **Motor Driver**
- **L293D** – dual H-Bridge for left and right DC motor control.

---

## 🧠 Features

- ⚙️ **Dual Mode Operation** — switch between digital (ESP32 + PID algorithm) and analog (discrete op-amp) control
- 🎛️ **PID Steering** — real-time error calculation from line sensors with motor speed correction
- 🔢 **8-Channel Multiplexer (MUX)** — efficient sensor array reading with minimal I/O pins
- 🧭 **Intersection & Finish Detection** — dedicated logic for intersections, dashed lines, and FINISH zone
- 🎚️ **Auto Sensor Calibration** — automatic black/white threshold calibration
- 🔄 **Motor Bias Diagnostics** — correction for left/right motor asymmetry (software & hardware)

---

## 🔌 Wiring Diagram

📌 *Coming soon: circuit diagrams and connection schematics.*

---

## 📸 Gallery

<p align="center">
  <img src="Dokumentasi/Start%20Merah.gif" width="400"/>
</p>
<p align="center">
  <em>Figure 1: Robot navigating the red-marked starting line</em>
</p>

<p align="center">
  <img src="Dokumentasi/Start%20Kuning.gif" width="400"/>
</p>
<p align="center">
  <em>Figure 2: Robot navigating the yellow-marked starting line</em>
</p>

<p align="center">
  <img src="Dokumentasi/LF%20Analog.gif" width="400"/>
</p>
<p align="center">
  <em>Figure 3: Robot running in Analog mode on the competition track</em>
</p>

---

## 🧩 Struktur Repository

```
Line-Follower-Analog-dan-Digital/
├── Firmware/
│   ├── DigitalMode/          # Kode ESP32 (PID digital, MUX, deteksi track)
│   └── AnalogMode/           # Skematik & kode pendukung rangkaian analog
├── Hardware/
│   ├── Schematics/           # Skematik rangkaian (op-amp, driver motor, dll)
│   └── PCB/                  # Desain PCB (jika ada)
├── Dokumentasi/               # Foto & GIF demo robot
├── README.md
└── LICENSE
```

---

## 🛠️ Tools & Libraries

- Arduino IDE / PlatformIO
- ESP32 Core & PID library / custom PWM logic
- L293D Motor Driver
- Op-amp LM358 (analog PID/comparator circuit)

---

## 🚀 Cara Penggunaan

1. Clone repository ini
   ```bash
   git clone https://github.com/username/Line-Follower-Analog-dan-Digital.git
   ```
2. Pilih mode operasi:
   - **Digital:** upload firmware dari folder `Firmware/DigitalMode/` ke ESP32
   - **Analog:** rakit rangkaian sesuai skematik di `Hardware/Schematics/`
3. Kalibrasi sensor sebelum robot dijalankan (`blackMode()` / `whiteMode()`)
4. Jalankan robot pada lintasan

---

## 📚 Future Improvements

- ⚙️ Tuning otomatis parameter PID (auto-tuning)
- 📊 Logging data performa digital vs analog untuk perbandingan langsung
- 🔋 Optimasi konsumsi daya

---

## 🙌 Credits

**Team Members:**
- Dafi Khalif Arrafa — D-IV Teknologi Rekayasa Instrumentasi dan Kontrol, Sekolah Vokasi UGM
- Rio Daris Syathir — D-IV Teknologi Rekayasa Instrumentasi dan Kontrol, Sekolah Vokasi UGM
- Dilla Zulfahrani — D-IV Teknologi Rekayasa Instrumentasi dan Kontrol, Sekolah Vokasi UGM

---

## 📄 Lisensi

Project ini menggunakan lisensi [MIT License](LICENSE).
