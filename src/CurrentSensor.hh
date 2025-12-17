#pragma once
#include <Arduino.h>

class CurrentSensor {
private:
    int sensorPin;             // ADC-Pin für Strommessung
    float offsetVoltage;       // Offset in Volt (z. B. 1.65 V)
    float vRef;                // ADC-Referenzspannung
    int adcResolution;         // z. B. 10 Bit → 1023
    float sensitivity;         // Sensor-Empfindlichkeit (V/A)
    unsigned int samples;      // Anzahl der Messpunkte
    unsigned int sampleDelay;  // Abtastintervall (µs)

public:
    CurrentSensor();

    void begin(int pin, float offset = 1.65, float vref = 3.3, int adcRes = 1023);
    float readVoltageOnce();
    void recordSamples_VC();
    int measurePredictPause(uint32_t measure_ms, uint32_t pause_ms);
   
};
