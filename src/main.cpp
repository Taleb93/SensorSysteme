#include <Arduino.h>
#include <EloquentTinyML.h>
#include "device_model_data.h"

// ===== Modellparameter =====
#define NUMBER_OF_INPUTS   7
#define NUMBER_OF_OUTPUTS  3
#define TENSOR_ARENA_SIZE  8192

Eloquent::TinyML::TfLite<
  NUMBER_OF_INPUTS,
  NUMBER_OF_OUTPUTS,
  TENSOR_ARENA_SIZE
> ml;

// Reihenfolge MUSS exakt dem Training entsprechen
const char* LABELS[NUMBER_OF_OUTPUTS] = {
  "loetstation",
  "netzteil1",
  "ventilator"
};

int argmax(const float* v, int n) {
  int best = 0;
  for (int i = 1; i < n; i++) {
    if (v[i] > v[best]) best = i;
  }
  return best;
}

void setup() {
  Serial.begin(9600);
  delay(1500);

  Serial.println();
  Serial.println("=== TinyML Device Classifier ===");
  Serial.print("Model size (bytes): ");
  Serial.println(device_model_tflite_len);

  // ✅ korrekt für EloquentTinyML 0.0.3
  if (!ml.begin(device_model_tflite)) {
    Serial.println("❌ ml.begin() failed (arena too small?)");
    while (true) delay(1000);
  }

  Serial.println("✅ Model loaded");
const float NORM_MEAN[7] = {
  0.22044146f,
  0.72652048f,
  0.10979843f,
  0.06079911f,
  0.05950867f,
  0.03682167f,
  0.04685974f
};
const float NORM_STD[7] = {
  0.14829580f,
  0.74155492f,
  0.03814124f,
  0.03673674f,
  0.06318647f,
  0.04406127f,
  0.06101553f
};

  // ===== Feature-Vektor (7 Features) =====
//   float x_raw[NUMBER_OF_INPUTS] = {
//   0.1065133f,
//   0.2379f,
//   0.07361594f,
//   0.03341273f,
//   0.03463272f,
//   0.01434687f,
//   0.008635364f
// };
 float x_raw[NUMBER_OF_INPUTS] = {0.1378424045785621,0.2056,0.1037486442014174,0.0457563991873742,0.0003035977727911,0.0009108647531416,0.001356505138266};
float x[NUMBER_OF_INPUTS];

// ✅ IDENTISCHE Normalisierung wie in Python
for (int i = 0; i < NUMBER_OF_INPUTS; i++) {
  x[i] = (x_raw[i] - NORM_MEAN[i]) / NORM_STD[i];
}

float y[NUMBER_OF_OUTPUTS];

// Inferenz
ml.predict(x, y);

Serial.println("Probabilities (normalized input):");
for (int i = 0; i < NUMBER_OF_OUTPUTS; i++) {
  Serial.print(LABELS[i]);
  Serial.print(": ");
  Serial.println(y[i], 6);
}

int pred = argmax(y, NUMBER_OF_OUTPUTS);
Serial.print("✅ Predicted device: ");
Serial.println(LABELS[pred]);

}

void loop() {
  delay(2000);
}