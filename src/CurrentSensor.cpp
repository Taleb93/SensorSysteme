#include "CurrentSensor.hh"
#include "feature.hh"

 
#define DEFAULT_SAMPLES 240
#define DEFAULT_DELAY_US 333 // ≈ 3 kHz Abtastrate
static const int NUM_CLASSES = 4;
CurrentSensor::CurrentSensor() {
    samples = DEFAULT_SAMPLES;
    sampleDelay = DEFAULT_DELAY_US;
    sensitivity = 0.4;
}

void CurrentSensor::begin(int pin, float offset, float vref, int adcRes) {
    sensorPin = pin;
    offsetVoltage = offset;
    vRef = vref;
    adcResolution = adcRes;
    analogReadResolution(10);
    Serial.println("🔌 CurrentSensor initialisiert...");
}

float CurrentSensor::readVoltageOnce() {
    int raw = analogRead(sensorPin);
    float voltage = (raw / (float)adcResolution) * vRef;
    return voltage;
}
void CurrentSensor::recordSamples_VC() {
  static unsigned long t_buf[DEFAULT_SAMPLES];
    static float v_buf[DEFAULT_SAMPLES];
    static float c_buf[DEFAULT_SAMPLES];

    const unsigned long Ts_us = sampleDelay; // gewünschte Periode

    unsigned long next_t = micros();  // Startzeit

    for (unsigned int i = 0; i < samples; i++) {

        // Warte bis zum geplanten Zeitpunkt
        while ((long)(micros() - next_t) < 0) {
            // busy-wait
        }

        unsigned long t = micros();
        float v = readVoltageOnce();
        float c = (v - offsetVoltage) / sensitivity;

        t_buf[i] = t;
        v_buf[i] = v;
        c_buf[i] = c;

        next_t += Ts_us;  // nächstes Sample planen
    }
    for (unsigned int i = 0; i < samples; i++) {
        Serial.print(t_buf[i]);
        Serial.print("\t");
        Serial.print(v_buf[i], 4);
        Serial.print("\t");
        Serial.println(c_buf[i], 4);
    }
}
void CurrentSensor::recordFeatureVectors_4s_20vec() {

  const unsigned long Ts_us = sampleDelay;
  unsigned long next_t = micros();

  FeatureExtractor fx;
  fx.begin(Ts_us);

  // 4s / 20ms = 200 Fenster
  // 200 Fenster / 20 Vektoren = 10 Fenster pro Vektor
  const int windowsPerVector = 10;
  const int outVectors = 20;
  const int featN = 7;

  float vec[outVectors][featN];
  for (int v = 0; v < outVectors; v++)
    for (int i = 0; i < featN; i++)
      vec[v][i] = 0.0f;

  int currentVec = 0;
  int winInVec = 0;

  unsigned long t_end = micros() + 4000UL * 1000UL;

  while ((long)(micros() - t_end) < 0 && currentVec < outVectors) {

    while ((long)(micros() - next_t) < 0) {}
    next_t += Ts_us;

    float v = readVoltageOnce();
    float c = (v - offsetVoltage) / sensitivity;

    fx.addSample(c);

    if (fx.windowReady()) {
      float feats[featN];
      if (fx.computeFeatures(feats)) {

        // Features aufsummieren
        for (int i = 0; i < featN; i++) {
          vec[currentVec][i] += feats[i];
        }
        winInVec++;

        // 10 Fenster voll -> Mittelwert bilden -> nächster Vektor
        if (winInVec >= windowsPerVector) {
          for (int i = 0; i < featN; i++) {
            vec[currentVec][i] /= (float)windowsPerVector;
          }
          currentVec++;
          winInVec = 0;
        }
      }
      fx.resetWindow();
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

extern int predictClass(float *x);
int CurrentSensor::measurePredictPause(uint32_t measure_ms, uint32_t pause_ms) {

  const unsigned long Ts_us = sampleDelay;
  unsigned long next_t = micros();

  FeatureExtractor fx;
  fx.begin(Ts_us);

  int votes[NUM_CLASSES] = {0};

  // Features über die Messung mitteln
  float featSum[7] = {0};
  int featCount = 0;

  unsigned long t_end = micros() + (unsigned long)measure_ms * 1000UL;

  while ((long)(micros() - t_end) < 0) {

    while ((long)(micros() - next_t) < 0) {}
    next_t += Ts_us;

    float v = readVoltageOnce();
    float c = (v - offsetVoltage) / sensitivity;

    fx.addSample(c);

    if (fx.windowReady()) {
      float feats[7];
      if (fx.computeFeatures(feats)) {

        // 1) Features sammeln (roh)
        for (int i = 0; i < 7; i++) featSum[i] += feats[i];
        featCount++;

        // 2) Predict (wie bisher) -> Voting
        float x[7];
        for (int i = 0; i < 7; i++) {
          x[i] = (feats[i] - NORM_MEAN[i]) / NORM_STD[i];
        }
        int pred = predictClass(x);
        if (pred >= 0 && pred < NUM_CLASSES) votes[pred]++;
      }
      fx.resetWindow();
    }
  }

  // Voting Ergebnis
  int best = 0;
  for (int i = 1; i < NUM_CLASSES; i++)
    if (votes[i] > votes[best]) best = i;

  // ---- Feature-Vektor pro Messung ausgeben ----
  Serial.println("---- Measurement features (avg over windows) ----");
  if (featCount > 0) {
    float avgFeat[7];
    for (int i = 0; i < 7; i++) avgFeat[i] = featSum[i] / (float)featCount;

    Serial.print("I_rms_A=");   Serial.print(avgFeat[0], 6); Serial.print("\t");
    Serial.print("I_max_A=");   Serial.print(avgFeat[1], 6); Serial.print("\t");
    Serial.print("Amp_50Hz=");  Serial.print(avgFeat[2], 6); Serial.print("\t");
    Serial.print("Amp_100Hz="); Serial.print(avgFeat[3], 6); Serial.print("\t");
    Serial.print("Amp_150Hz="); Serial.print(avgFeat[4], 6); Serial.print("\t");
    Serial.print("Amp_200Hz="); Serial.print(avgFeat[5], 6); Serial.print("\t");
    Serial.print("Amp_250Hz="); Serial.println(avgFeat[6], 6);
  } else {
    Serial.println("No windows computed (featCount=0)");
  }

  delay(pause_ms);
  return best;
}
