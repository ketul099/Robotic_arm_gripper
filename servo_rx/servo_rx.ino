#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

// NRF24: keep away from servo pins for stability
RF24 radio(8, 10); // CE=8 CSN=10
const byte address[6] = "FLEX1";

struct FlexPayload {
  uint16_t flexNorm[5]; // 0..1000
  uint32_t ms;
  uint8_t  calibrated;
};
FlexPayload payload;

Servo servos[5];
const uint8_t servoPins[5] = {3, 4, 5, 6, 7};

// Per-finger servo limits (tune for your hand mechanics)
uint8_t servoMin[5] = {10, 10, 10, 10, 10};
uint8_t servoMax[5] = {170,170,170,170,170};

const uint32_t TIMEOUT_MS = 500;
uint32_t lastRx = 0;
bool linkActive = false;

int map1000ToAngle(uint16_t x, uint8_t outMin, uint8_t outMax) {
  if (x > 1000) x = 1000;
  long v = (long)x * (outMax - outMin) / 1000L + outMin;
  if (v < outMin) v = outMin;
  if (v > outMax) v = outMax;
  return (int)v;
}

void neutral() {
  for (uint8_t i = 0; i < 5; i++) servos[i].write(90);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  for (uint8_t i = 0; i < 5; i++) {
    servos[i].attach(servoPins[i]);
    servos[i].write(90);
  }

  if (!radio.begin()) {
    while (1) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(120);
    }
  }

  radio.setPALevel(RF24_PA_LOW);
  radio.setDataRate(RF24_250KBPS);
  radio.setChannel(108);
  radio.openReadingPipe(1, address);
  radio.startListening();

  lastRx = millis();
  Serial.println("\n=== SERVO RX (normalized) ===");
}

void loop() {
  if (radio.available()) {
    radio.read(&payload, sizeof(payload));
    lastRx = millis();

    if (!linkActive) {
      linkActive = true;
      Serial.println("LINK OK");
    }

    for (uint8_t i = 0; i < 5; i++) {
      int ang = map1000ToAngle(payload.flexNorm[i], servoMin[i], servoMax[i]);
      servos[i].write(ang);
    }

    digitalWrite(LED_BUILTIN, HIGH);

    static uint32_t cnt = 0;
    cnt++;
    if (cnt % 50 == 0) {
      Serial.print("RX norm: ");
      for (uint8_t i = 0; i < 5; i++) {
        Serial.print(payload.flexNorm[i]);
        Serial.print(i < 4 ? "," : "");
      }
      Serial.print(" | Lat=");
      Serial.print((uint32_t)(millis() - payload.ms));
      Serial.print("ms | Cal=");
      Serial.println(payload.calibrated ? "YES" : "NO");
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (millis() - lastRx > TIMEOUT_MS) {
    if (linkActive) {
      linkActive = false;
      Serial.println("LINK LOST -> FAILSAFE");
      neutral();
    }
    digitalWrite(LED_BUILTIN, (millis() / 500) & 1);
  }
}
