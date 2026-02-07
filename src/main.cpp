#include <Arduino.h>
#include <EloquentTinyML.h>
#include "device_model_int8_floatio_data.h"

#include "CurrentSensor.hh"
#include "LoRaE5.hh"
#include "norm_params.hh"

// ===== Modellparameter =====
#define NUMBER_OF_INPUTS   7
#define NUMBER_OF_OUTPUTS  4
#define TENSOR_ARENA_SIZE  8192

HardwareSerial LoRaSerial(USART3);
LoRaE5 sensor(LoRaSerial);

Eloquent::TinyML::TfLite<
  NUMBER_OF_INPUTS,
  NUMBER_OF_OUTPUTS,
  TENSOR_ARENA_SIZE
> ml;

const char* LABELS[NUMBER_OF_OUTPUTS] = {
  "noDevice",       // id 0
  "Lötstation",     // id 1
  "Ventilator",     // id 2
  "Laptopnetzteil"  // id 3
};

int argmax(const float* v, int n) {
  int best = 0;
  for (int i = 1; i < n; i++) {
    if (v[i] > v[best]) best = i;
  }
  return best;
}

int predictClass(float *x) {          
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
  Serial.println(model_int8_floatio_tflite_len);

  if (!ml.begin(model_int8_floatio_tflite)) {
    Serial.println(" ml.begin() failed (arena too small?)");
    while (true) delay(1000);
  }
  Serial.println("Model loaded");
  cs.begin(A0, 1.65f, 3.3f, 1023);
  Serial.println("Sensor ready");
}

void loop() {
   int pred = cs.measurePredictPause(2000, 3000);
   Serial.print("Predicted device: ");
   Serial.println(LABELS[pred]);
   sensor.SendDetectedDevice_kompakt(LABELS[pred]);

}
