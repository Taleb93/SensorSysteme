// feature.cpp
#include "feature.hh"                                 // FeatureExtractor
#include <math.h>                                     // Mathe-Funktionen
#ifndef M_PI
#define M_PI 3.14159265358979323846                   // Pi-Konstante
#endif
FeatureExtractor::FeatureExtractor(float period_s) : PERIOD_S(period_s) {} // Fensterzeit setzen

void FeatureExtractor::begin(uint32_t sampleDelayUs, size_t maxBuffer) {
  Ts_us = sampleDelayUs;                              // Sample-Abstand (µs)
  dt = (float)Ts_us * 1e-6f;                          // Sample-Abstand (s)
  fs = 1.0f / dt;                                     // Abtastrate (Hz)

  maxN = (maxBuffer > 256) ? 256 : maxBuffer;         // Buffer-Limit
  buf = staticBuf;                                    // interner Puffer

  long nCalc = lroundf(PERIOD_S * fs);                // N aus Fensterzeit
  if (nCalc < 8) nCalc = 8;                           // Mindest-N
  if ((size_t)nCalc > maxN) nCalc = (long)maxN;       // auf Buffer begrenzen
  N = (size_t)nCalc;                                  // Fensterlänge

  buildHamming();                                     // Hamming-Fenster bauen
  resetWindow();                                      // Fenster reset
}

void FeatureExtractor::buildHamming() {
  // np.hamming(N): w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1)) // Hamming-Formel
  if (N < 2) return;                                  // zu klein
  for (size_t n = 0; n < N; n++) {
    w[n] = 0.54f - 0.46f * cosf((2.0f * (float)M_PI * (float)n) / (float)(N - 1)); // Gewichte
  }
}

void FeatureExtractor::resetWindow() {
  idx = 0;                                            // Index reset
  sumSq = 0.0f;                                       // RMS-Summe reset
  maxAbs = 0.0f;                                      // Max reset
}

void FeatureExtractor::addSample(float current_A) {
  if (idx >= N) return;                               // Fenster voll

  buf[idx] = current_A;                               // Sample speichern
  sumSq += current_A * current_A;                     // Quadratsumme
  float a = fabsf(current_A);                         // Betrag
  if (a > maxAbs) maxAbs = a;                         // Maximum tracken
  idx++;                                              // Index erhöhen
}

bool FeatureExtractor::windowReady() const {
  return idx >= N;                                    // Fenster bereit?
}

size_t FeatureExtractor::nearestBinIndex(float f_hz) const {
  // rfftfreq Näherung                              // Bin-Näherung
  long k = lroundf(f_hz * (float)N * dt);             // k ≈ f*N*dt
  if (k < 0) k = 0;                                   // min clamp
  long kmax = (long)(N / 2);                          // Nyquist clamp
  if (k > kmax) k = kmax;                             // max clamp
  return (size_t)k;                                   // Index zurück
}

float FeatureExtractor::goertzelMagBin(const float* x, size_t N, size_t k) const {
  if (N == 0) return 0.0f;                            // Schutz
  if (k == 0) {                                       // DC-Bin Fall
    float re = 0.0f;                                  // Realteil
    for (size_t n = 0; n < N; n++) re += x[n];        // Summe bilden
    return fabsf(re);                                 // Betrag zurück
  }

  float omega = (2.0f * (float)M_PI * (float)k) / (float)N; // Winkel
  float coeff = 2.0f * cosf(omega);                   // Goertzel Koeff

  float s_prev = 0.0f;                                // Zustand 1
  float s_prev2 = 0.0f;                               // Zustand 2

  for (size_t n = 0; n < N; n++) {
    float s = x[n] + coeff * s_prev - s_prev2;        // Goertzel Rekurrenz
    s_prev2 = s_prev;                                 // shift
    s_prev = s;                                       // shift
  }

  float real = s_prev - s_prev2 * cosf(omega);        // Realteil
  float imag = s_prev2 * sinf(omega);                 // Imaginärteil

  return sqrtf(real * real + imag * imag);            // Betrag DFT[k]
}

bool FeatureExtractor::computeFeatures(float features[FEATURE_COUNT]) {
  if (!windowReady() || N == 0) return false;         // nur wenn ready

  float meanSq = sumSq / (float)N;                    // Mittelwert Quadrat
  float I_rms = sqrtf(meanSq);                        // RMS berechnen
  float I_max = maxAbs;                               // Peak übernehmen

  float mean = 0.0f;                                  // Mittelwert
  for (size_t n = 0; n < N; n++) mean += buf[n];      // Summe bilden
  mean /= (float)N;                                   // durch N teilen

  static float xw[256];                               // Fenster-Array
  for (size_t n = 0; n < N; n++) {
    xw[n] = (buf[n] - mean) * w[n];                   // AC + Hamming
  }

  features[0] = I_rms;                                // Feature 0
  features[1] = I_max;                                // Feature 1

  for (int i = 0; i < FREQ_COUNT; i++) {
    size_t k = nearestBinIndex((float)freqs[i]);      // nächster Bin
    float mag = goertzelMagBin(xw, N, k) * (2.0f / (float)N); // Betrag normiert
    features[2 + i] = mag;                            // Spektral-Feature
  }

  return true;                                        // OK
}
