#include "CurrentSensor.hh"

#define DEFAULT_SAMPLES 240
#define DEFAULT_DELAY_US 333 // ≈ 3 kHz Abtastrate

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

    Serial.println("Zeit (us)\tSpannung (V)\tStrom (A)");
    for (unsigned int i = 0; i < samples; i++) {
        Serial.print(t_buf[i]);
        Serial.print("\t");
        Serial.print(v_buf[i], 4);
        Serial.print("\t");
        Serial.println(c_buf[i], 4);
    }
}

// RMS-Berechnung
float CurrentSensor::measureRMS() {
    float sumSq = 0.0;
    for (unsigned int i = 0; i < samples; i++) {
        float c = readCurrentOnce();
        sumSq += c * c;
        delayMicroseconds(sampleDelay);
    }
    float rms = sqrt(sumSq / samples);
    return rms;
}
