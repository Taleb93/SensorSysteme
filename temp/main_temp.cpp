#include <Arduino.h>
#include "CurrentSensor.hh"

// 👉 Hier nur den ADC-Pin angeben:
#define CURRENT_SENSOR_PIN A0

CurrentSensor sensor;

void setup() {
  Serial.begin(115200);
  sensor.begin(CURRENT_SENSOR_PIN);

  Serial.println("== Stromsensor-Test (Nucleo-F303RE) ==");
  delay(2000);
}

void loop() {
  // Alle 2 Sekunden RMS-Wert messen
  float rms = sensor.measureRMS();
  Serial.print("Gemessener RMS-Strom: ");
  Serial.print(rms, 3);
  Serial.println(" A");

  delay(2000);
}
