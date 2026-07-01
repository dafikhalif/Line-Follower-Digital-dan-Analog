// ======================================================
// LINE FOLLOWER - ESP32
// Auto-calibration + blackMode/whiteMode/sharpTurn
// ======================================================

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ======================================================
// OLED
// ======================================================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ======================================================
// MUX
// ======================================================
#define MUX_Z_PIN   34
#define MUX_S0_PIN  25
#define MUX_S1_PIN  26
#define MUX_S2_PIN  27

// ======================================================
// MOTOR
// ======================================================
#define PWM_KIRI   18
#define PWM_KANAN  19
const int freqPWM       = 1000;
const int resolutionPWM = 8;

// ======================================================
// BUTTON
// ======================================================
#define BTN_MODE   13
#define BTN_UP     14
#define BTN_DOWN   32
#define BTN_START  33

// ======================================================
// BUZZER
// ======================================================
#define BUZZER_PIN 23

// ======================================================
// KALIBRASI
// ======================================================
int  calMin[8];
int  calMax[8];
int  thresholds[8];
bool isCalibrated = false;
int  calState     = 0;

// ======================================================
// PARAMETER
// ======================================================
int   SPEED_BASE = 105;
float Kp         = 2.0;
float Ki         = 0.0001;
float Kd         = 2.1;
float scaler     = 24;

float previousError = 0;
float integral      = 0;

unsigned long lastSharpTurnTime       = 0;
const unsigned long sharpTurnCooldown = 30;

// ======================================================
// FINISH
// ======================================================
#define FINISH_THRESHOLD   150   // ~150ms konsisten baru dianggap finish (anti false trigger di gap)
#define FINISH_MIN_RUNTIME 2000
int           finishCount = 0;
bool          finished    = false;
unsigned long startTime   = 0;

// ======================================================
// PERSIMPANGAN X
// ======================================================
#define INTERSECTION_TIMEOUT 80
bool inIntersection    = false;
int  intersectionCount = 0;

// ======================================================
// TRACK MODE (mode-aware, otomatis)
// ======================================================
enum TrackMode { MODE_BLACK, MODE_WHITE };
TrackMode currentTrackMode = MODE_BLACK;

// ======================================================
// DUAL LINE CORRIDOR (dua garis putus-putus sejajar)
// ======================================================
float dualLineErrorSmooth = 0;

// ======================================================
// STATE
// ======================================================
int  menuIndex = 0;
bool robotRun  = false;

bool lastModeState  = HIGH;
bool lastUpState    = HIGH;
bool lastDownState  = HIGH;
bool lastStartState = HIGH;

// ======================================================
// FORWARD DECLARE
// ======================================================
int  readSensor(int channel);
void motor(int kiri, int kanan);
void beep(int jumlah);
void tampilRunning(int activeCount, const char* mode);
void tampilMenu();
void tampilFinish();
void tampilCalibrationMenu();
void doCalibration(bool isWhite);
void bacaButton();
void blackMode(int sensorVals[]);
void whiteMode(int sensorVals[]);
void PID(float weightedSum, int sum, const char* mode);
void sharpTurn(int sensorVals[]);
bool frontSensorDetecting();
bool isActive(int i, int sensorVals[], TrackMode mode);
int  countActive(int sensorVals[], TrackMode mode);
bool detectDualLine(int sensorVals[], TrackMode mode);

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  pinMode(MUX_S0_PIN, OUTPUT);
  pinMode(MUX_S1_PIN, OUTPUT);
  pinMode(MUX_S2_PIN, OUTPUT);
  pinMode(MUX_Z_PIN,  INPUT);

  pinMode(BTN_MODE,  INPUT_PULLUP);
  pinMode(BTN_UP,    INPUT_PULLUP);
  pinMode(BTN_DOWN,  INPUT_PULLUP);
  pinMode(BTN_START, INPUT_PULLUP);

  pinMode(BUZZER_PIN, OUTPUT);

  ledcAttach(PWM_KIRI,  freqPWM, resolutionPWM);
  ledcAttach(PWM_KANAN, freqPWM, resolutionPWM);

  motor(0, 0);

  for (int i = 0; i < 8; i++) {
    calMin[i]     = 4095;
    calMax[i]     = 0;
    thresholds[i] = 2048;
  }

  tampilCalibrationMenu();
  beep(2);
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  bacaButton();

  if (!isCalibrated) {
    tampilCalibrationMenu();
    delay(30);
    return;
  }

  if (!robotRun) {
    tampilMenu();
    delay(30);
    return;
  }

  if (finished) {
    tampilFinish();
    delay(50);
    return;
  }

  if (startTime == 0) startTime = millis();

  // ====================================================
  // BACA SENSOR
  // ====================================================
  int sensorVals[8];
  for (int i = 0; i < 8; i++) sensorVals[i] = readSensor(i);

  // ====================================================
  // TENTUKAN MODE TRACK (mode-aware)
  // Garis selalu MINORITAS sensor (background dominan),
  // jadi mode dipilih berdasarkan sisi mana yang lebih sedikit aktif.
  // ====================================================
  int rawBlackCount = 0;
  for (int i = 0; i < 8; i++) {
    if (sensorVals[i] > thresholds[i]) rawBlackCount++;
  }
  int rawWhiteCount = 8 - rawBlackCount;

  if (rawBlackCount < rawWhiteCount) currentTrackMode = MODE_BLACK;
  else                                currentTrackMode = MODE_WHITE;

  int activeBlackCount = countActive(sensorVals, currentTrackMode);

  // ====================================================
  // FINISH (mode-aware)
  // ====================================================
  if (activeBlackCount >= 7 &&
      millis() - startTime > FINISH_MIN_RUNTIME) {
    finishCount++;
    if (finishCount > FINISH_THRESHOLD) {
      motor(0, 0);
      finished = true;
      robotRun = false;
      beep(3);
      return;
    }
  } else {
    finishCount = 0;
  }

  // ====================================================
  // DUAL LINE CORRIDOR (dua garis putus-putus sejajar kiri-kanan)
  // ====================================================
  bool isDualLine = detectDualLine(sensorVals, currentTrackMode);

  if (isDualLine) {
    // Jangan percaya weightedSum mentah (rawan oscillate karena dash
    // kiri & kanan biasanya gak sinkron fase). Smoothing + gain dikecilin.
    float rawError = previousError;
    dualLineErrorSmooth = dualLineErrorSmooth * 0.85 + rawError * 0.15;

    int adjustment = (int)(dualLineErrorSmooth * scaler * 0.3);
    int lSpd = constrain(SPEED_BASE + adjustment, 0, 255);
    int rSpd = constrain((int)((SPEED_BASE - adjustment) * 0.97), 0, 255);
    motor(lSpd, rSpd);

    tampilRunning(activeBlackCount, "DUAL-LINE");
    delay(1);
    return;
  } else {
    // reset smoothing saat keluar dari pola dual-line
    dualLineErrorSmooth = previousError;
  }

  // ====================================================
  // SHARP TURN (mode-aware)
  // ====================================================
  sharpTurn(sensorVals);

  // ====================================================
  // PERSIMPANGAN X (mode-aware)
  // ====================================================
  bool leftSide  = isActive(0, sensorVals, currentTrackMode) ||
                   isActive(1, sensorVals, currentTrackMode);
  bool rightSide = isActive(6, sensorVals, currentTrackMode) ||
                   isActive(7, sensorVals, currentTrackMode);
  bool center    = isActive(3, sensorVals, currentTrackMode) ||
                   isActive(4, sensorVals, currentTrackMode);

  if (leftSide && rightSide && center && activeBlackCount >= 3) {
    inIntersection    = true;
    intersectionCount = 0;
  }

  if (inIntersection) {
    intersectionCount++;
    float output     = Kp * previousError;
    int   adjustment = (int)(output * scaler * 0.5);
    int   lSpd       = constrain(SPEED_BASE + adjustment, 0, 255);
    int   rSpd       = constrain((int)((SPEED_BASE - adjustment) * 0.97), 0, 255);
    motor(lSpd, rSpd);

    if (intersectionCount > INTERSECTION_TIMEOUT ||
        activeBlackCount <= 2) {
      inIntersection    = false;
      intersectionCount = 0;
    }

    tampilRunning(activeBlackCount, "X-CROSS");
    delay(1);
    return;
  }

  // ====================================================
  // MODE SELECTOR (garis hitam vs putih, berdasarkan currentTrackMode)
  // ====================================================
  if (currentTrackMode == MODE_WHITE) {
    whiteMode(sensorVals);
  } else if (activeBlackCount > 0) {
    blackMode(sensorVals);
  } else {
    int adj  = (int)(Kp * previousError * scaler * 0.5);
    int lSpd = constrain(75 + adj, 0, 255);
    int rSpd = constrain((int)((75 - adj) * 0.97), 0, 255);
    motor(lSpd, rSpd);
    tampilRunning(0, "LOST");
  }

  delay(1);
}

// ======================================================
// MODE-AWARE HELPERS
// ======================================================
bool isActive(int i, int sensorVals[], TrackMode mode) {
  if (mode == MODE_BLACK) return sensorVals[i] > thresholds[i];
  else                    return sensorVals[i] <= thresholds[i];
}

int countActive(int sensorVals[], TrackMode mode) {
  int c = 0;
  for (int i = 0; i < 8; i++) {
    if (isActive(i, sensorVals, mode)) c++;
  }
  return c;
}

// Dua garis putus-putus sejajar (kiri & kanan aktif, tengah kosong)
bool detectDualLine(int sensorVals[], TrackMode mode) {
  bool leftActive  = isActive(0, sensorVals, mode) || isActive(1, sensorVals, mode);
  bool rightActive = isActive(6, sensorVals, mode) || isActive(7, sensorVals, mode);
  bool centerEmpty = !isActive(3, sensorVals, mode) && !isActive(4, sensorVals, mode);

  return leftActive && rightActive && centerEmpty;
}

// ======================================================
// BLACK MODE — garis hitam bg putih
// ======================================================
void blackMode(int sensorVals[]) {
  float weights[8]  = { -10, -5, -2, -0.5, 0.5, 2, 5, 10 };
  float weightedSum = 0;
  int   sum         = 0;

  for (int i = 0; i < 8; i++) {
    int active   = (sensorVals[i] > thresholds[i]) ? 1000 : 0;
    weightedSum += active * weights[i];
    sum         += active;
    Serial.print(sensorVals[i]); Serial.print("\t");
  }
  Serial.println();

  PID(weightedSum, sum, "BLACK");
}

// ======================================================
// WHITE MODE — garis putih putus-putus bg hitam
// Full PID, balik logika sensor
// ======================================================
void whiteMode(int sensorVals[]) {
  float weights[8]  = { -10, -5, -2, -0.5, 0.5, 2, 5, 10 };
  float weightedSum = 0;
  int   sum         = 0;

  for (int i = 0; i < 8; i++) {
    // Putih = ADC rendah -> balik logika
    int active   = (sensorVals[i] <= thresholds[i]) ? 1000 : 0;
    weightedSum += active * weights[i];
    sum         += active;
    Serial.print(sensorVals[i]); Serial.print("\t");
  }
  Serial.println();

  // Gap putus-putus (sum==0) -> pakai previousError otomatis di PID
  PID(weightedSum, sum, "WHITE");
}

// ======================================================
// PID
// ======================================================
void PID(float weightedSum, int sum, const char* mode) {
  float error = (sum != 0) ? (weightedSum / sum) : previousError;

  integral        += error;
  float derivative = error - previousError;
  float output     = Kp * error + Ki * integral + Kd * derivative;
  previousError    = error;

  int adjustment       = (int)(output * scaler);
  int turnSlowdown     = abs(adjustment) / 3;
  int dynamicBaseSpeed = constrain(SPEED_BASE - turnSlowdown, 50, SPEED_BASE);

  int leftSpeed  = constrain(dynamicBaseSpeed + adjustment, 0, 255);
  int rightSpeed = constrain((int)((dynamicBaseSpeed - adjustment) * 0.97), 0, 255);

  motor(leftSpeed, rightSpeed);
  tampilRunning(sum / 1000, mode);

  Serial.print(mode);
  Serial.print(" ERR:"); Serial.print(error);
  Serial.print(" L:");   Serial.print(leftSpeed);
  Serial.print(" R:");   Serial.println(rightSpeed);
}

// ======================================================
// SHARP TURN (mode-aware)
// ======================================================
void sharpTurn(int sensorVals[]) {
  unsigned long now = millis();
  if (now - lastSharpTurnTime < sharpTurnCooldown) return;
  if (inIntersection) return;

  bool leftDetected  = isActive(0, sensorVals, currentTrackMode) ||
                       isActive(1, sensorVals, currentTrackMode);
  bool rightDetected = isActive(6, sensorVals, currentTrackMode) ||
                       isActive(7, sensorVals, currentTrackMode);
  bool centerClear   = !isActive(3, sensorVals, currentTrackMode) &&
                       !isActive(4, sensorVals, currentTrackMode);

  if (leftDetected && !rightDetected && centerClear) {
    Serial.println("Sharp LEFT");
    while (!frontSensorDetecting()) motor(0, 120);
    lastSharpTurnTime = millis();

  } else if (rightDetected && !leftDetected && centerClear) {
    Serial.println("Sharp RIGHT");
    while (!frontSensorDetecting()) motor(120, 0);
    lastSharpTurnTime = millis();
  }
}

// ======================================================
// FRONT SENSOR (mode-aware)
// ======================================================
bool frontSensorDetecting() {
  for (int i = 2; i <= 5; i++) {
    int val = readSensor(i);
    bool active = (currentTrackMode == MODE_BLACK) ? (val > thresholds[i])
                                                     : (val <= thresholds[i]);
    if (active) return true;
  }
  return false;
}

// ======================================================
// KALIBRASI
// ======================================================
void doCalibration(bool isWhite) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(isWhite ? "CAL: PUTIH" : "CAL: HITAM");
  display.println("Geser robot 3 detik");
  display.display();

  unsigned long start = millis();
  while (millis() - start < 3000) {
    for (int i = 0; i < 8; i++) {
      int val = readSensor(i);
      if (isWhite) { if (val < calMin[i]) calMin[i] = val; }
      else         { if (val > calMax[i]) calMax[i] = val; }
    }
    delay(10);
  }

  if (!isWhite) {
    for (int i = 0; i < 8; i++) {
      thresholds[i] = (calMin[i] + calMax[i]) / 2;
      Serial.print("thr["); Serial.print(i);
      Serial.print("]="); Serial.println(thresholds[i]);
    }
    isCalibrated = true;
  }

  beep(1);
}

void tampilCalibrationMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("KALIBRASI");
  display.println("");
  if (calState == 0) {
    display.println("START: Cal PUTIH");
    display.println("(taruh di background");
    display.println(" putih)");
  } else if (calState == 1) {
    display.println("START: Cal HITAM");
    display.println("(taruh di garis");
    display.println(" hitam)");
  }
  display.display();
}

// ======================================================
// READ SENSOR
// ======================================================
int readSensor(int channel) {
  digitalWrite(MUX_S0_PIN,  channel        & 0x01);
  digitalWrite(MUX_S1_PIN, (channel >> 1)  & 0x01);
  digitalWrite(MUX_S2_PIN, (channel >> 2)  & 0x01);
  delayMicroseconds(10);
  analogRead(MUX_Z_PIN);
  delayMicroseconds(10);
  int total = 0;
  for (int j = 0; j < 5; j++) {
    total += analogRead(MUX_Z_PIN);
    delayMicroseconds(2);
  }
  return total / 5;
}

// ======================================================
// MOTOR
// ======================================================
void motor(int kiri, int kanan) {
  ledcWrite(PWM_KIRI,  constrain(kiri,  0, 255));
  ledcWrite(PWM_KANAN, constrain(kanan, 0, 255));
}

// ======================================================
// BUTTON
// ======================================================
void bacaButton() {
  bool modeState  = digitalRead(BTN_MODE);
  bool upState    = digitalRead(BTN_UP);
  bool downState  = digitalRead(BTN_DOWN);
  bool startState = digitalRead(BTN_START);

  if (lastStartState == HIGH && startState == LOW) {
    if (!isCalibrated) {
      if (calState == 0) {
        doCalibration(true);
        calState = 1;
      } else if (calState == 1) {
        doCalibration(false);
        calState = 2;
      }
    } else {
      robotRun           = !robotRun;
      finished           = false;
      finishCount        = 0;
      integral           = 0;
      previousError       = 0;
      inIntersection      = false;
      intersectionCount   = 0;
      dualLineErrorSmooth = 0;
      startTime           = 0;
      motor(0, 0);
      beep(2);
    }
  }

  if (isCalibrated) {
    if (lastModeState == HIGH && modeState == LOW) {
      menuIndex++;
      if (menuIndex > 3) menuIndex = 0;
      beep(1);
    }
    if (lastUpState == HIGH && upState == LOW) {
      if (menuIndex == 0) Kp         += 0.1;
      if (menuIndex == 1) Ki         += 0.01;
      if (menuIndex == 2) Kd         += 0.1;
      if (menuIndex == 3) SPEED_BASE += 2;
      beep(1);
    }
    if (lastDownState == HIGH && downState == LOW) {
      if (menuIndex == 0) Kp         -= 0.1;
      if (menuIndex == 1) Ki         -= 0.01;
      if (menuIndex == 2) Kd         -= 0.1;
      if (menuIndex == 3) SPEED_BASE -= 2;
      beep(1);
    }
  }

  lastModeState  = modeState;
  lastUpState    = upState;
  lastDownState  = downState;
  lastStartState = startState;
  delay(20);
}

// ======================================================
// OLED
// ======================================================
void tampilMenu() {
  display.clearDisplay();
  display.setCursor(0, 0); display.println("SETTING");
  display.setCursor(0, 16);
  if (menuIndex == 0) display.print("> "); else display.print("  ");
  display.print("KP: "); display.println(Kp);
  if (menuIndex == 1) display.print("> "); else display.print("  ");
  display.print("KI: "); display.println(Ki, 4);
  if (menuIndex == 2) display.print("> "); else display.print("  ");
  display.print("KD: "); display.println(Kd);
  if (menuIndex == 3) display.print("> "); else display.print("  ");
  display.print("SPD:"); display.println(SPEED_BASE);
  display.display();
}

void tampilRunning(int activeCount, const char* mode) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("SPD:"); display.print(SPEED_BASE);
  display.print(" N:"); display.println(activeCount);
  display.print("KP:"); display.print(Kp);
  display.print(" KD:"); display.println(Kd);
  display.setCursor(0, 24);
  display.println(mode);
  display.setCursor(0, 40);
  for (int i = 0; i < 8; i++) {
    int val = readSensor(i);
    bool active = (currentTrackMode == MODE_BLACK) ? (val > thresholds[i])
                                                     : (val <= thresholds[i]);
    display.print(active ? "1 " : "0 ");
  }
  display.display();
}

void tampilFinish() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(20, 20); display.println("FINISH!");
  display.setTextSize(1);
  display.setCursor(10, 50); display.println("Press START to reset");
  display.display();
}

// ======================================================
// BUZZER
// ======================================================
void beep(int jumlah) {
  for (int i = 0; i < jumlah; i++) {
    digitalWrite(BUZZER_PIN, HIGH); delay(50);
    digitalWrite(BUZZER_PIN, LOW);  delay(50);
  }
}