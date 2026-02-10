#pragma once
#include <Arduino.h>
#include <stddef.h>

class FeatureExtractor {
public:
  static const int FEATURE_COUNT = 7;
  static const int FREQ_COUNT = 5;

  // PERIOD_S = 0.020s, FREQ_TARGETS = {50,100,150,200,250}
  FeatureExtractor(float period_s = 0.020f);

  // sampleDelayUs z.B. 333us => ~3kHz (wie in deinem CurrentSensor) :contentReference[oaicite:1]{index=1}
  void begin(uint32_t sampleDelayUs, size_t maxBuffer = 256);

  // pro Sample aufrufen (gleichmäßige Abtastung!)
  void addSample(float current_A);

  bool windowReady() const;
  void resetWindow();

  // features = [I_rms_A, I_max_A, Amp_50, Amp_100, Amp_150, Amp_200, Amp_250]
  // Return: true wenn berechnet, false wenn Fenster nicht voll.
  bool computeFeatures(float features[FEATURE_COUNT]);

  size_t windowSize() const { return N; }
  float sampleRateHz() const { return fs; }

private:
  float PERIOD_S;
  uint32_t Ts_us = 0;
  float dt = 0.0f;
  float fs = 0.0f;

  // Fensterlänge N wie Python: N = round(PERIOD_S*fs), N>=8
  size_t N = 0;
  size_t maxN = 256;

  // Targets
  int freqs[FREQ_COUNT] = {50, 100, 150, 200, 250};

  // Buffer
  float *buf = nullptr;          // zeigt auf statischen Puffer unten
  float staticBuf[256];

  // Hamming window
  float w[256];

  // Laufvariablen für schnelle RMS / MaxAbs
  size_t idx = 0;
  float sumSq = 0.0f;
  float maxAbs = 0.0f;

  void buildHamming();
  float goertzelMagBin(const float* x, size_t N, size_t k) const;
  size_t nearestBinIndex(float f_hz) const; // wie np.argmin(|freqs - f|)
};
