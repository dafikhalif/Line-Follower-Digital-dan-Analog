<div align="center">

# 🏁 Line Follower — Analog & Digital PID Control

**Robot line follower dengan dua sistem kendali independen: PID digital berbasis ESP32 dan PID analog berbasis op-amp.**

![Status](https://img.shields.io/badge/status-active-brightgreen)
![Platform](https://img.shields.io/badge/platform-ESP32-blue)
![License](https://img.shields.io/badge/license-MIT-yellow)
![Mode](https://img.shields.io/badge/mode-Analog%20%7C%20Digital-orange)

<br/>

<p align="center">
  <img src="Dokumentasi/Start%20Merah.gif" width="32%" />
  <img src="Dokumentasi/Start%20Kuning.gif" width="32%" />
  <img src="Dokumentasi/LF%20Analog.gif" width="32%" />
</p>

</div>

---

## 📑 Daftar Isi

- [Tentang Project](#-tentang-project)
- [Digital vs Analog](#️-digital-vs-analog)
- [Fitur Utama](#-fitur-utama)
- [Sistem & Hardware](#-sistem--hardware)
- [Skematik Rangkaian](#-skematik-rangkaian)
- [Cara Penggunaan](#-cara-penggunaan)
- [Rencana Pengembangan](#-rencana-pengembangan)
- [Tim](#-tim)

---

## 🎯 Tentang Project

Project ini dibangun untuk mengeksplorasi dua pendekatan kendali PID pada robot line follower secara berdampingan dalam satu platform yang sama:

- **Mode Digital** — logika PID sepenuhnya diproses oleh mikrokontroler ESP32, memungkinkan tuning parameter secara fleksibel dan penambahan logika kompleks (deteksi persimpangan, garis putus-putus, zona FINISH).
- **Mode Analog** — kendali PID direalisasikan murni lewat rangkaian op-amp **TL074 & TL082**, tanpa pemrosesan digital, menghasilkan respons kontrol yang jauh lebih cepat namun kurang fleksibel untuk logika kompleks.

Kombinasi ini dipakai untuk riset perbandingan performa kendali PID digital vs analog, sekaligus untuk kebutuhan kompetisi robotika.

---

## ⚖️ Digital vs Analog

| Aspek                     | 🔵 Mode Digital (ESP32)              | 🟠 Mode Analog (Op-Amp TL074/TL082)   |
|----------------------------|---------------------------------------|-----------------------------------------|
| Basis kendali              | Algoritma PID di firmware             | Rangkaian PID diskrit                   |
| Kecepatan respons           | Bergantung siklus loop program        | Real-time, nyaris tanpa delay           |
| Fleksibilitas logika        | Tinggi (persimpangan, FINISH, dll)    | Terbatas pada steering dasar            |
| Kemudahan tuning            | Ubah parameter via kode               | Ubah nilai komponen (resistor/kapasitor)|
| Kompleksitas hardware       | Lebih sederhana (1 board utama)       | Lebih banyak komponen diskrit           |

---

## ✨ Fitur Utama

- ⚙️ **Dual Mode Operation** — beralih antara kendali digital dan analog pada satu chassis yang sama
- 🎛️ **PID Steering** — kalkulasi error dari array sensor garis dengan koreksi kecepatan motor real-time
- 🔢 **8-Channel Multiplexer (MUX)** — pembacaan array sensor Super Bright LED & Photodiode secara efisien dengan jumlah pin minimal
- 🧭 **Deteksi Persimpangan & FINISH** — logika khusus untuk intersection, garis putus-putus, dan zona akhir lintasan (mode digital)
- 🎚️ **Auto-Kalibrasi Sensor** — kalibrasi otomatis threshold hitam/putih (`blackMode()` / `whiteMode()`)
- 🔄 **Diagnostik Bias Motor** — koreksi asimetri kecepatan motor kiri/kanan

---

## 🧩 Sistem & Hardware

**Sensor Array**
- 8 pasang **Super Bright LED & Photodiode** untuk pembacaan garis
- Multiplexer 8-channel untuk efisiensi jalur sinyal ke pengendali

**Unit Kendali Digital**
- ESP32 sebagai otak utama, menjalankan algoritma PID
- Auto-kalibrasi threshold hitam/putih
- Logika deteksi persimpangan, garis putus-putus, dan zona FINISH

**Unit Kendali Analog**

Rangkaian PID analog dibangun murni dari op-amp (TL074 & TL082), tersusun dari beberapa blok berikut:

- **VREF Generator** — pembangkit tegangan referensi sebagai titik tengah (midpoint) sistem
- **Summing Point Sensor** — menjumlahkan sinyal dari 8 sensor IR melalui potensiometer pembobot menjadi dua sinyal gabungan (kiri dan kanan)
- **Blok Error** — menghitung selisih antara sinyal kiri dan kanan terhadap tegangan referensi untuk mendapatkan sinyal error
- **Blok P, I, D** — tiga rangkaian op-amp terpisah yang mengolah sinyal error menjadi komponen Proportional, Integral, dan Derivative, masing-masing dengan potensiometer gain untuk tuning manual
- **Summing PID** — menjumlahkan ketiga komponen P, I, D menjadi satu sinyal kendali gabungan
- **Blok Motor Mixer** — menggabungkan sinyal kendali dengan tegangan referensi untuk menghasilkan dua sinyal output analog sebagai perintah kecepatan motor kanan & kiri

**Konektor**
- Konektor sensor — input 8 kanal sensor IR dari array
- Konektor digital — meneruskan sinyal output analog beserta suplai daya menuju board digital (ESP32) untuk dibaca ADC dan dikonversi menjadi PWM motor
- Distribusi daya — menyalurkan suplai 5V ke seluruh op-amp pada rangkaian

**Motor Driver**
- L293D dual H-Bridge untuk motor kiri dan kanan

---

## 🖼️ Skematik Rangkaian

**Mainboard (Digital & Analog)**

<img src="Skematik/Skematik%20Digital.png" width="800" />

**Rangkaian PID Analog (Op-Amp)**

<img src="Skematik/Skematik%20Analog.png" width="800" />

---

## 🚀 Cara Penggunaan

1. Clone repository ini
   ```bash
   git clone https://github.com/username/Line-Follower-Analog-dan-Digital.git
   ```
2. Pilih mode operasi:
   - **Digital** → upload firmware dari `Firmware/DigitalMode/` ke ESP32
   - **Analog** → rakit rangkaian sesuai skematik di `Skematik/`
3. Kalibrasi sensor sebelum robot dijalankan (`blackMode()` / `whiteMode()`)
4. Jalankan robot pada lintasan

---

## 📚 Rencana Pengembangan

- ⚙️ Auto-tuning parameter PID
- 📊 Logging data performa untuk membandingkan respons digital vs analog secara kuantitatif
- 🔋 Optimasi konsumsi daya

---

## 👥 Tim

- **Dafi Khalif Arrafa** — D-IV Teknologi Rekayasa Instrumentasi dan Kontrol, Sekolah Vokasi UGM
- **Rio Daris Syathir** — D-IV Teknologi Rekayasa Instrumentasi dan Kontrol, Sekolah Vokasi UGM
- **Dilla Zulfahrani** — D-IV Teknologi Rekayasa Instrumentasi dan Kontrol, Sekolah Vokasi UGM

---

<div align="center">

📄 Project ini menggunakan lisensi [MIT License](LICENSE).

</div>
