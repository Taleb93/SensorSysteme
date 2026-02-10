#include "CurrentSensor.hh"
#include "feature.hh"

#define DEFAULT_SAMPLES 240
#define DEFAULT_DELAY_US 333 // ~3 kHz Sampling
static const int NUM_CLASSES = 4;

CurrentSensor::CurrentSensor() {
    samples = DEFAULT_SAMPLES;      // default sample count
    sampleDelay = DEFAULT_DELAY_US; // sampling period (µs)
    sensitivity = 0.4;              // sensor gain factor
}

void CurrentSensor::begin(int pin, float offset, float vref, int adcRes) {
    sensorPin = pin;           // ADC input pin
    offsetVoltage = offset;    // DC offset level
    vRef = vref;               // ADC reference voltage
    adcResolution = adcRes;    // ADC max value
    analogReadResolution(10);  // set ADC bits
    Serial.println("🔌 CurrentSensor initialisiert..."); // status print
}

float CurrentSensor::readVoltageOnce() {
    int raw = analogRead(sensorPin);                 // read ADC raw
    float voltage = (raw / (float)adcResolution) * vRef; // raw→voltage
    return voltage;                                  // return voltage
}

void CurrentSensor::recordSamples_VC() {
    static unsigned long t_buf[DEFAULT_SAMPLES]; // time buffer
    static float v_buf[DEFAULT_SAMPLES];         // voltage buffer
    static float c_buf[DEFAULT_SAMPLES];         // current buffer

    const unsigned long Ts_us = sampleDelay; // target period
    unsigned long next_t = micros();         // next sample time

    for (unsigned int i = 0; i < samples; i++) {

        while ((long)(micros() - next_t) < 0) {
            // busy-wait timing
        }

        unsigned long t = micros();                 // timestamp
        float v = readVoltageOnce();                // sample voltage
        float c = (v - offsetVoltage) / sensitivity; // voltage→current

        t_buf[i] = t; // store time
        v_buf[i] = v; // store voltage
        c_buf[i] = c; // store current

        next_t += Ts_us; // schedule next sample
    }

    for (unsigned int i = 0; i < samples; i++) {
        Serial.print(t_buf[i]);     // print time
        Serial.print("\t");
        Serial.print(v_buf[i], 4);  // print voltage
        Serial.print("\t");
        Serial.println(c_buf[i], 4); // print current
    }
}
// CurrentSensor.cpp
void CurrentSensor::recordFeatureVectors_20vec() {
  const unsigned long Ts_us = sampleDelay;           // Sample-Periode
  unsigned long next_t = micros();                   // nächste Sample-Zeit
  FeatureExtractor fx;                               // Feature-Objekt
  fx.begin(Ts_us);                                   // Feature-Init
  // 4s / 20ms = 200 Fenster                         // Fenster-Rechnung
  // 200 Fenster / 20 Vektoren = 10 Fenster/Vektor   // Vektor-Rechnung
  const int windowsPerVector = 10;                   // Fenster pro Vektor
  const int outVectors = 20;                         // Anzahl Vektoren
  const int featN = 7;                               // Feature-Anzahl
  float vec[outVectors][featN];                      // Vektor-Puffer
  for (int v = 0; v < outVectors; v++)               // init v
    for (int i = 0; i < featN; i++)                  // init i
      vec[v][i] = 0.0f;                              // auf 0 setzen
  int currentVec = 0;                                // aktueller Vektor
  int winInVec = 0;                                  // Fenster-Zähler
  unsigned long t_end = micros() + 4000UL * 1000UL;   // Messdauer 4s

  while ((long)(micros() - t_end) < 0 && currentVec < outVectors) { // Loop bis fertig

    while ((long)(micros() - next_t) < 0) {}          // busy-wait Timing
    next_t += Ts_us;                                  // nächstes Sample
    float v = readVoltageOnce();                      // Spannung lesen
    float c = (v - offsetVoltage) / sensitivity;      // Strom berechnen
    fx.addSample(c);                                  // Sample hinzufügen
    if (fx.windowReady()) {                           // Fenster voll?
      float feats[featN];                             // Feature-Puffer
      if (fx.computeFeatures(feats)) {                // Features berechnen

        for (int i = 0; i < featN; i++) {             // Features aufsummieren
          vec[currentVec][i] += feats[i];             // Summe pro Feature
        }
        winInVec++;                                   // Fenster zählen

        if (winInVec >= windowsPerVector) {           // 10 Fenster erreicht?
          for (int i = 0; i < featN; i++) {           // Mittelwert bilden
            vec[currentVec][i] /= (float)windowsPerVector; // durch 10 teilen
          }
          currentVec++;                               // nächster Vektor
          winInVec = 0;                               // Reset Zähler
        }
      }
      fx.resetWindow();                               // Fenster reset
    }
  }

  // Ausgabe: NUR 20 Vektoren als CSV
  Serial.println("=== NOLOAD_FEATURES_BEGIN ===");
  Serial.println("idx,I_rms_A,I_max_A,Amp_50Hz,Amp_100Hz,Amp_150Hz,Amp_200Hz,Amp_250Hz");

  for (int v = 0; v < currentVec; v++) {
    Serial.print(v); Serial.print(",");
    for (int i = 0; i < featN; i++) {
      Serial.print(vec[v][i], 6);
      if (i < featN - 1) Serial.print(",");
    }
    Serial.println();
  }

  Serial.println("=== NOLOAD_FEATURES_END ===");
}
#include "norm_params.hh"


// CurrentSensor.cpp

extern int predictClass(float *x);                  // ML-Funktion extern

int CurrentSensor::measurePredictPause(uint32_t measure_ms, uint32_t pause_ms) {

  const unsigned long Ts_us = sampleDelay;          // Sample-Periode
  unsigned long next_t = micros();                  // nächste Sample-Zeit

  FeatureExtractor fx;                              // Feature-Objekt
  fx.begin(Ts_us);                                  // init mit Ts

  int votes[NUM_CLASSES] = {0};                     // Voting-Zähler

  float featSum[7] = {0};                           // Feature-Summe
  int featCount = 0;                                // Fenster-Anzahl

  unsigned long t_end = micros() + (unsigned long)measure_ms * 1000UL; // Mess-Ende

  while ((long)(micros() - t_end) < 0) {            // messen bis Ende

    while ((long)(micros() - next_t) < 0) {}        // busy-wait Timing
    next_t += Ts_us;                                // nächstes Sample planen

    float v = readVoltageOnce();                    // Spannung lesen
    float c = (v - offsetVoltage) / sensitivity;    // Strom berechnen

    fx.addSample(c);                                // Sample ins Fenster

    if (fx.windowReady()) {                         // Fenster voll?
      float feats[7];                               // Feature-Puffer
      if (fx.computeFeatures(feats)) {              // Features berechnen

        for (int i = 0; i < 7; i++) featSum[i] += feats[i]; // Features aufsummieren
        featCount++;                                 // Fenster zählen

        float x[7];                                  // Norm-Input
        for (int i = 0; i < 7; i++) {
          x[i] = (feats[i] - NORM_MEAN[i]) / NORM_STD[i];   // z-Score Norm
        }
        int pred = predictClass(x);                  // Klasse vorhersagen
        if (pred >= 0 && pred < NUM_CLASSES) votes[pred]++; // Vote zählen
      }
      fx.resetWindow();                              // Fenster zurücksetzen
    }
  }

  int best = 0;                                      // beste Klasse
  for (int i = 1; i < NUM_CLASSES; i++)              // Mehrheitswahl
    if (votes[i] > votes[best]) best = i;

  Serial.println("---- Measurement features (avg over windows) ----"); // Header
  if (featCount > 0) {                              // Fenster vorhanden?
    float avgFeat[7];                               // Mittelwert-Features
    for (int i = 0; i < 7; i++) avgFeat[i] = featSum[i] / (float)featCount; // Mittelwert

    Serial.print("I_rms_A=");   Serial.print(avgFeat[0], 6); Serial.print("\t");   // RMS
    Serial.print("I_max_A=");   Serial.print(avgFeat[1], 6); Serial.print("\t");   // Peak
    Serial.print("Amp_50Hz=");  Serial.print(avgFeat[2], 6); Serial.print("\t");   // 50 Hz
    Serial.print("Amp_100Hz="); Serial.print(avgFeat[3], 6); Serial.print("\t");   // 100 Hz
    Serial.print("Amp_150Hz="); Serial.print(avgFeat[4], 6); Serial.print("\t");   // 150 Hz
    Serial.print("Amp_200Hz="); Serial.print(avgFeat[5], 6); Serial.print("\t");   // 200 Hz
    Serial.print("Amp_250Hz="); Serial.println(avgFeat[6], 6);                     // 250 Hz
  } else {
    Serial.println("No windows computed (featCount=0)");     // keine Fenster
  }

  delay(pause_ms);                                 // Pause nach Messung
  return best;                                     // Ergebnis zurückgeben
}
