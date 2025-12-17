#include "CurrentSensor.hh"
#include "feature.hh"
#include "norm_params.hh"

extern int predictClass(float *x);
 
#define DEFAULT_SAMPLES 240
#define DEFAULT_DELAY_US 333 // ≈ 3 kHz Abtastrate
static const int NUM_CLASSES = 3;
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
int CurrentSensor::measurePredictPause(uint32_t measure_ms, uint32_t pause_ms) {

  const unsigned long Ts_us = sampleDelay;
  unsigned long next_t = micros();

  FeatureExtractor20ms fx;
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
