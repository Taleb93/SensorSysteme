// main.cpp

#include <Arduino.h>                         // Arduino Basis
#include <EloquentTinyML.h>                  // TinyML Wrapper
#include "device_model_int8_floatio_data.h"  // TFLite Modell

#include "CurrentSensor.hh"                  // Strommessung + Features
#include "LoRaE5.hh"                         // LoRaWAN Kommunikation
#include "norm_params.hh"                    // Norm-Parameter

// ===== Modellparameter =====
#define NUMBER_OF_INPUTS   7                 // 7 Features
#define NUMBER_OF_OUTPUTS  4                 // 4 Klassen
#define TENSOR_ARENA_SIZE  8192              // RAM für TFLite

HardwareSerial LoRaSerial(USART3);           // UART3 für LoRa
LoRaE5 lora(LoRaSerial);                     // LoRa-E5 Objekt
CurrentSensor sensor;                        // Sensor Objekt

Eloquent::TinyML::TfLite<
  NUMBER_OF_INPUTS,
  NUMBER_OF_OUTPUTS,
  TENSOR_ARENA_SIZE
> ml;                                        // ML Inferenz-Engine

const char* LABELS[NUMBER_OF_OUTPUTS] = {    // Klassen-Namen
  "noDevice",       // id 0
  "Lötstation",     // id 1
  "Ventilator",     // id 2
  "Laptopnetzteil"  // id 3
};

int argmax(const float* v, int n) {          // größtes Element finden
  int best = 0;
  for (int i = 1; i < n; i++) {
    if (v[i] > v[best]) best = i;            // bestes Logit/Score
  }
  return best;                               // Index zurückgeben
}

int predictClass(float *x) {                 // Feature → Klasse
  float y[NUMBER_OF_OUTPUTS];                // Output-Puffer
  ml.predict(x, y);                          // Inferenz ausführen
  return argmax(y, NUMBER_OF_OUTPUTS);       // beste Klasse
}

void setup() {
  Serial.begin(9600);                        // Debug UART
  lora.begin(9600);                          // LoRa UART init
  lora.joinNetwork();                        // LoRaWAN Join

  delay(1500);                               // Start stabilisieren
  Serial.println();                          // Leerzeile
  Serial.println("=== TinyML Device Classifier (LIVE) ==="); // Titel
  Serial.print("Model size (bytes): ");      // Modellgröße
  Serial.println(model_int8_floatio_tflite_len);

  if (!ml.begin(model_int8_floatio_tflite)) { // Modell laden
    Serial.println(" ml.begin() failed (arena too small?)");  // Fehlerhinweis
    while (true) delay(1000);                // Stop (safe)
  }
  Serial.println("Model loaded");            // OK Meldung

  sensor.begin(A0, 1.65f, 3.3f, 1023);       // ADC + Offset setup
  Serial.println("Sensor ready");            // Sensor OK
}

void loop() {
  int pred = sensor.measurePredictPause(2000, 3000); // messen+klassifizieren
  Serial.print("Predicted device: ");        // Ausgabe-Label
  Serial.println(LABELS[pred]);              // Klasse drucken
  lora.SendDetectedDevice(LABELS[pred]);     // Klasse senden
}