#include <Arduino.h>
#include "LedControl.hh"
#include <iostream>
LedControl led(LED_BUILTIN); // interne LED des NUCLEO
HardwareSerial LoRaSerial(3);  // UART3: TX=PD8, RX=PD9

void setup() {
  Serial.begin(9600);        // USB-Serial (zum PC)
  LoRaSerial.begin(9600);    // UART für LoRa-Modul

  Serial.println("=== LoRa-E5 Test ===");
  delay(1000);

  Serial.println("Sende 'AT' an das LoRa-Modul...");
  LoRaSerial.println("AT");   // Testbefehl
}

void loop() {
  // Wenn das LoRa-Modul antwortet
  if (LoRaSerial.available()) {
    String antwort = LoRaSerial.readStringUntil('\n');
    Serial.print("Antwort vom LoRa: ");
    Serial.println(antwort);
  }
}
