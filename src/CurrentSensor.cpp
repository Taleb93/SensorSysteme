#include "CurrentSensor.hh"

#define DEFAULT_SAMPLES 240
#define DEFAULT_DELAY_US 333 // ≈ 3 kHz Abtastrate

CurrentSensor::CurrentSensor() {
    samples = DEFAULT_SAMPLES;
    sampleDelay = DEFAULT_DELAY_US;
    sensitivity = 0.2; // durch Spannungsteiler halbiert (0.4 V/A → 0.2 V/A)

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
float CurrentSensor::readCurrentOnce() {
    int raw = analogRead(sensorPin);
    float voltage = (raw / (float)adcResolution) * vRef;
    float current = (voltage - offsetVoltage) / sensitivity;
    return current;
}
// Messreihe: Zeit, Spannung & Strom ausgeben (für MATLAB oder Python)
void CurrentSensor::recordSamples_VC() {
    Serial.println("Zeit (us)\tSpannung (V)\tStrom (A)");

    for (unsigned int i = 0; i < samples; i++) {
        unsigned long t = micros();
        float v = readVoltageOnce();
        float c = (v - offsetVoltage) / sensitivity;

        Serial.print(t);
        Serial.print("\t");
        Serial.print(v, 4);
        Serial.print("\t");
        Serial.println(c, 4);

        delayMicroseconds(sampleDelay);
    }
}


// Messreihe: Zeit- und Stromwerte ausgeben (für MATLAB)
void CurrentSensor::recordSamples() {
    float current[samples];
    unsigned long timeStamp[samples];

    Serial.println("Zeit (us)\tStrom (A)");

    for (unsigned int i = 0; i < samples; i++) {
        timeStamp[i] = micros();
        current[i] = readCurrentOnce();
        Serial.print(timeStamp[i]);
        Serial.print("\t");
        Serial.println(current[i], 3);
        delayMicroseconds(sampleDelay);
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
