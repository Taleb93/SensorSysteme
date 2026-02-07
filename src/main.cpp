#include <Arduino.h>
#include <EloquentTinyML.h>
#include "device_model_data.h"

#include "CurrentSensor.hh"
#include "LoRaE5.hh"
#include "norm_params.hh"

// ===== Modellparameter =====
#define NUMBER_OF_INPUTS   7
#define NUMBER_OF_OUTPUTS  3
#define TENSOR_ARENA_SIZE  8192

HardwareSerial LoRaSerial(USART3);
LoRaE5 sensor(LoRaSerial);

Eloquent::TinyML::TfLite<
  NUMBER_OF_INPUTS,
  NUMBER_OF_OUTPUTS,
  TENSOR_ARENA_SIZE
> ml;

const char* LABELS[NUMBER_OF_OUTPUTS] = {
  "Loetstation",
  "Laptopnetzteil",
  "Ventilator"
};

int argmax(const float* v, int n) {
  int best = 0;
  for (int i = 1; i < n; i++) {
    if (v[i] > v[best]) best = i;
  }
  return best;
}

// ✅ genau wie dein main-Flow: ml.predict(x,y) + argmax
int predictClass(float *x) {          // <-- NICHT const
  float y[NUMBER_OF_OUTPUTS];
  ml.predict(x, y);
  return argmax(y, NUMBER_OF_OUTPUTS);
}


CurrentSensor cs;

void setup() {
  Serial.begin(9600);
 
  sensor.begin(9600);
  sensor.joinNetwork();
    
 delay(1500);
  Serial.println();
  Serial.println("=== TinyML Device Classifier (LIVE) ===");
  Serial.print("Model size (bytes): ");
  Serial.println(device_model_tflite_len);

  if (!ml.begin(device_model_tflite)) {
    Serial.println("❌ ml.begin() failed (arena too small?)");
    while (true) delay(1000);
  }
  Serial.println("✅ Model loaded");

  // Sensor init (Beispielwerte anpassen!)
  // offsetVoltage: 1.65V bei ACS712 + 3.3V Versorgung
  cs.begin(A0, 1.65f, 3.3f, 1023);

  Serial.println("✅ Sensor ready");
}

void loop() {
  
  // 2s messen -> 1 Ergebnis -> 3s Pause -> wiederholen
  cs.recordSamples_VC();
  // int pred = cs.measurePredictPause(2000, 3000);
  // Serial.print("✅ Predicted device: ");
  // Serial.println(LABELS[pred]);
  // sensor.SendDetectedDevice_kompakt(LABELS[pred]);

  // Wenn du Labels statt Zahl willst: du kannst in CurrentSensor
  // best zurückgeben oder dort Serial.println(LABELS[best]) machen.
}
