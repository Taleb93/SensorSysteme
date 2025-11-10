//#include <Arduino.h>
#include "LoRaE5.hh"
#include "DistanceSensor.hh"
Distance_Sensor Abstand_Sensor;
HardwareSerial LoRaSerial(USART3);
LoRaE5 sensor(LoRaSerial);
const int trigPin = 9;
const int echoPin = 10;
float  distance;
#define sendPin D7
#define busyPin D8
#define falidPin D6
void setup() {
     Abstand_Sensor.sensorInit(10,9);
     Serial.begin(9600);
     sensor.begin(9600);
     sensor.joinNetwork();
     pinMode(sendPin,OUTPUT);
     pinMode(busyPin,OUTPUT);
     pinMode(falidPin,OUTPUT);   
}

void loop() {
    distance = Abstand_Sensor.getDistance(100);
    static unsigned long lastSend = 0;
    unsigned long now = millis();
    if (now - lastSend > 10000) {  // alle 10 Sekunden
    sensor.readAndSendTemperatureWithDistance(distance);
    sensor.control_ledLink(sendPin,busyPin,falidPin);
    lastSend = now;
}
}
