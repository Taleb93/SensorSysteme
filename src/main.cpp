#include <Arduino.h>
#include "CurrentSensor.hh"

#define CURRENT_SENSOR_PIN A0

CurrentSensor sensor;

void setup() {
  Serial.begin(9600);
  sensor.begin(CURRENT_SENSOR_PIN);
  Serial.println("== Stromsensor-Test (Nucleo-F303RE) ==");
  delay(2000);
}

void loop() {
  // Alle 2 Sekunden RMS-Wert messen
  // float rms = sensor.measureRMS();
  // Serial.print("Gemessener RMS-Strom: ");
  // Serial.print(rms, 3);
  // Serial.println(" A");
  // float vol = sensor.readVoltageOnce();
  // Serial.print("Gemessener Spannug: ");
  // Serial.print(vol, 3);
  // Serial.println(" V");
  sensor.recordSamples_VC();

}
