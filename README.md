# 🤖 Line Follower Analog dan Digital

Robot line follower dengan **dua mode kendali**: mode **Digital** (PID berbasis mikrokontroler ESP32) dan mode **Analog** (rangkaian op-amp berbasis komparator/PID analog LM358). Dikembangkan untuk kebutuhan riset PID control dan kompetisi robotika.

<p align="center">
  <img src="Dokumentasi/Start_Merah.gif" width="32%" />
  <img src="Dokumentasi/Start_Kuning.gif" width="32%" />
  <img src="Dokumentasi/LF_Analog.gif" width="32%" />
</p>

---

## ✨ Fitur

- ⚙️ **Dual Mode Operation** — dapat beroperasi dengan kendali digital (ESP32 + algoritma PID) atau kendali analog (rangkaian op-amp diskrit)
- 🎛️ **PID Steering** — kalkulasi error dari sensor garis dan koreksi kecepatan motor secara real-time
- 🔢 **8-Channel Multiplexer (MUX)** — pembacaan array sensor garis melalui MUX untuk efisiensi pin I/O
- 🧭 **Deteksi Persimpangan & Finish** — logika khusus untuk intersection, garis putus-putus, dan zona FINISH
- 🎚️ **Auto-Kalibrasi Sensor** — kalibrasi otomatis threshold hitam/putih (`blackMode()` / `whiteMode()`)
- 🔄 **Diagnostik Bias Motor** — koreksi asimetri motor kiri/kanan (perbaikan software & hardware)

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
├── image/                    # Foto & GIF demo robot
├── README.md
└── LICENSE
```

---

## 🛠️ Tools & Komponen

| Kategori         | Detail                                  |
|-------------------|------------------------------------------|
| Mikrokontroler    | ESP32                                    |
| Sensor Garis      | Array fototransistor/IR + MUX 8 channel  |
| Rangkaian Analog  | Op-amp LM358 (komparator/PID analog)     |
| Driver Motor      | (isi sesuai driver yang dipakai)         |
| Firmware          | Arduino / C++                            |
| Software Pendukung| Arduino IDE / PlatformIO                 |

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

## 📸 Dokumentasi

![Hardware](image/hardware.jpeg)

---

## 👤 Kontributor

- **Dafi Khalif Arrafa** — D-IV Instrumentasi dan Kontrol, Sekolah Vokasi UGM

---

## 📄 Lisensi

Project ini menggunakan lisensi [MIT License](LICENSE).
