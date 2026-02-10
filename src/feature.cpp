#include "feature.hh"
#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
FeatureExtractor::FeatureExtractor(float period_s) : PERIOD_S(period_s) {}

void FeatureExtractor::begin(uint32_t sampleDelayUs, size_t maxBuffer) {
  Ts_us = sampleDelayUs;
  dt = (float)Ts_us * 1e-6f;
  fs = 1.0f / dt;

  maxN = (maxBuffer > 256) ? 256 : maxBuffer;
  buf = staticBuf;

  long nCalc = lroundf(PERIOD_S * fs);
  if (nCalc < 8) nCalc = 8;
  if ((size_t)nCalc > maxN) nCalc = (long)maxN;
  N = (size_t)nCalc;

  buildHamming();
  resetWindow();
}
void FeatureExtractor::buildHamming() {
  // np.hamming(N): w[n] = 0.54 - 0.46*cos(2*pi*n/(N-1))
  if (N < 2) return;
  for (size_t n = 0; n < N; n++) {
    w[n] = 0.54f - 0.46f * cosf((2.0f * (float)M_PI * (float)n) / (float)(N - 1));
  }
}
void FeatureExtractor::resetWindow() {
  idx = 0;
  sumSq = 0.0f;
  maxAbs = 0.0f;
}
void FeatureExtractor::addSample(float current_A) {
  if (idx >= N) return;

  buf[idx] = current_A;
  sumSq += current_A * current_A;
  float a = fabsf(current_A);
  if (a > maxAbs) maxAbs = a;
  idx++;
}

bool FeatureExtractor::windowReady() const {
  return idx >= N;
}

size_t FeatureExtractor::nearestBinIndex(float f_hz) const {
  // Python: freqs = rfftfreq(N, d=dt) => freqs[k] = k/(N*dt)
  // idx = argmin(|freqs - f|) => entspricht round(f * N * dt)
  long k = lroundf(f_hz * (float)N * dt);
  if (k < 0) k = 0;
  long kmax = (long)(N / 2);
  if (k > kmax) k = kmax;
  return (size_t)k;
}

float FeatureExtractor::goertzelMagBin(const float* x, size_t N, size_t k) const {
  if (N == 0) return 0.0f;
  if (k == 0) {
    // DC-Bin (bei dir nicht genutzt), trotzdem korrekt:
    float re = 0.0f;
    for (size_t n = 0; n < N; n++) re += x[n];
    return fabsf(re);
  }

  float omega = (2.0f * (float)M_PI * (float)k) / (float)N;
  float coeff = 2.0f * cosf(omega);

  float s_prev = 0.0f;
  float s_prev2 = 0.0f;

  for (size_t n = 0; n < N; n++) {
    float s = x[n] + coeff * s_prev - s_prev2;
    s_prev2 = s_prev;
    s_prev = s;
  }

  float real = s_prev - s_prev2 * cosf(omega);
  float imag = s_prev2 * sinf(omega);

  return sqrtf(real * real + imag * imag); // = abs(DFT[k])
}

bool FeatureExtractor::computeFeatures(float features[FEATURE_COUNT]) {
  if (!windowReady() || N == 0) return false;

  // 1) I_rms und 2) I_max (genau wie Python)
  float meanSq = sumSq / (float)N;
  float I_rms = sqrtf(meanSq);
  float I_max = maxAbs;

  // mean(seg) für AC-Anteil
  float mean = 0.0f;
  for (size_t n = 0; n < N; n++) mean += buf[n];
  mean /= (float)N;

  // x[n] = (seg[n] - mean) * hamming[n]
  static float xw[256];
  for (size_t n = 0; n < N; n++) {
    xw[n] = (buf[n] - mean) * w[n];
  }

  features[0] = I_rms;
  features[1] = I_max;


  for (int i = 0; i < FREQ_COUNT; i++) {
    size_t k = nearestBinIndex((float)freqs[i]);
    float mag = goertzelMagBin(xw, N, k) * (2.0f / (float)N);
    features[2 + i] = mag;
  }

  return true;
}
