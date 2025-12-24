#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10);
const byte address[6] = "FLEX1";

const uint8_t flexPins[5] = {A0, A1, A2, A3, A4};
const uint8_t CALIB_BTN = 2;
const uint8_t STATUS_LED = 3;

struct FlexPayload {
  uint16_t flexNorm[5];  // 0..1000 normalized
  uint32_t ms;
  uint8_t  calibrated;  // 1 if calibrated
};
FlexPayload payload;

struct CalibData {
  uint16_t minVal[5];
  uint16_t maxVal[5];
  bool isCalibrated;
};
CalibData calib = {
  {1023,1023,1023,1023,1023},
  {0,0,0,0,0},
  false
};

const uint8_t AVG_N = 5;
uint16_t hist[5][AVG_N];
uint8_t idx = 0;

const uint8_t MAX_RETRIES = 3;
const uint8_t MAX_CONSEC_FAILS = 10;
uint32_t consecFails = 0;

uint16_t clampU16(uint16_t x, uint16_t a, uint16_t b) {
  if (x < a) return a;
  if (x > b) return b;
  return x;
}

uint16_t mapTo1000(uint16_t raw, uint16_t inMin, uint16_t inMax) {
  if (inMax <= inMin + 5) return 0; // avoid divide by tiny range
  raw = clampU16(raw, inMin, inMax);
  uint32_t num = (uint32_t)(raw - inMin) * 1000UL;
  uint32_t den = (uint32_t)(inMax - inMin);
  return (uint16_t)(num / den); // 0..1000
}

void initRadio() {
  if (!radio.begin()) {
    while (1) {
      digitalWrite(STATUS_LED, !digitalRead(STATUS_LED));
      delay(100);
    }
  }
  radio.setPALevel(RF24_PA_LOW);        // more stable than HIGH
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.setRetries(5, 15);
  radio.setAutoAck(true);
  radio.openWritingPipe(address);
  radio.stopListening();
}

void calibrateSensors() {
  Serial.println("\n>>> CALIBRATION MODE <<<");
  Serial.println("Move fingers full range, release button to finish (max 10s)");

  for (uint8_t i = 0; i < 5; i++) {
    calib.minVal[i] = 1023;
    calib.maxVal[i] = 0;
  }

  uint32_t start = millis();
  while (digitalRead(CALIB_BTN) == LOW && (millis() - start < 10000)) {
    for (uint8_t i = 0; i < 5; i++) {
      uint16_t raw = analogRead(flexPins[i]);
      if (raw < calib.minVal[i]) calib.minVal[i] = raw;
      if (raw > calib.maxVal[i]) calib.maxVal[i] = raw;
    }
    digitalWrite(STATUS_LED, (millis() / 200) & 1);
    delay(10);
  }
  digitalWrite(STATUS_LED, LOW);
  calib.isCalibrated = true;

  Serial.println("Sensor | Min | Max | Range");
  for (uint8_t i = 0; i < 5; i++) {
    Serial.print(i); Serial.print("      ");
    Serial.print(calib.minVal[i]); Serial.print("   ");
    Serial.print(calib.maxVal[i]); Serial.print("   ");
    Serial.println(calib.maxVal[i] - calib.minVal[i]);
  }
}

uint16_t readSmoothed(uint8_t ch) {
  uint16_t raw = analogRead(flexPins[ch]);
  hist[ch][idx] = raw;

  uint32_t sum = 0;
  for (uint8_t i = 0; i < AVG_N; i++) sum += hist[ch][i];
  return (uint16_t)(sum / AVG_N);
}

bool transmitData() {
  for (uint8_t a = 0; a < MAX_RETRIES; a++) {
    if (radio.write(&payload, sizeof(payload))) return true;
    delayMicroseconds(500);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  pinMode(CALIB_BTN, INPUT_PULLUP);
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, LOW);

  for (uint8_t ch = 0; ch < 5; ch++) {
    uint16_t seed = analogRead(flexPins[ch]);
    for (uint8_t i = 0; i < AVG_N; i++) hist[ch][i] = seed;
  }

  initRadio();
  Serial.println("\n=== FLEX TX (normalized) ===");
  Serial.println("Press button to calibrate");
}

void loop() {
  if (digitalRead(CALIB_BTN) == LOW) {
    calibrateSensors();
    return;
  }

  // Build payload
  for (uint8_t i = 0; i < 5; i++) {
    uint16_t raw = readSmoothed(i);

    // If not calibrated, use a safe default mapping
    uint16_t inMin = calib.isCalibrated ? calib.minVal[i] : 300;
    uint16_t inMax = calib.isCalibrated ? calib.maxVal[i] : 700;

    payload.flexNorm[i] = mapTo1000(raw, inMin, inMax);
  }
  payload.ms = millis();
  payload.calibrated = calib.isCalibrated ? 1 : 0;

  bool ok = transmitData();
  if (ok) {
    consecFails = 0;
    digitalWrite(STATUS_LED, HIGH); delay(2); digitalWrite(STATUS_LED, LOW);
  } else {
    consecFails++;
    digitalWrite(STATUS_LED, HIGH); delay(50); digitalWrite(STATUS_LED, LOW);
  }

  if (consecFails >= MAX_CONSEC_FAILS) {
    Serial.println("Re-init radio (too many fails)");
    initRadio();
    consecFails = 0;
  }

  static uint32_t lastPrint = 0;
  if (millis() - lastPrint > 1500) {
    Serial.print("Norm: ");
    for (uint8_t i = 0; i < 5; i++) {
      Serial.print(payload.flexNorm[i]);
      Serial.print(i < 4 ? "," : "");
    }
    Serial.print(" | Cal=");
    Serial.println(payload.calibrated ? "YES" : "NO");
    lastPrint = millis();
  }

  idx = (idx + 1) % AVG_N;
  delay(20);
}
